#include "chip8.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

// размер файла в байтах
long get_file_size(FILE *file)
{
    if (file)
    {
        fseek(file, 0, SEEK_END);     // переходим в конец файла
        long file_size = ftell(file); // узнаем, где находится указатель, чтобы понять какого размера файл
        rewind(file);                 // возращаемся назад
        return file_size;
    }
    else
    {
        perror("file: file is empty");
        return -1;
    }
}

int fontset_to_mem(uint8_t *memory)
{
    if (!memory)
    {
        return -1;
    }

    memcpy(&memory[0x050], fontset, sizeof(fontset));
    return 0;
}

void chip8_init(Chip8 *chip8)
{
    // зануляем массивы
    memset(chip8->memory, 0, MEMORY_SIZE);
    memset(chip8->V, 0, REGISTER_COUNT);
    memset(chip8->gfx, 0, sizeof(chip8->gfx));
    memset(chip8->stack, 0, sizeof(chip8->stack));
    memset(chip8->keypad, 0, KEY_COUNT);

    chip8->pc = 0x200; // код программы начинается именно с 0x200, поэтому считчек не на ноль, до 0x200 находятся шрифты и тп.
    chip8->I = 0;
    chip8->sp = 0;
    chip8->delay_timer = 0;
    chip8->sound_timer = 0;

    fontset_to_mem(chip8->memory); // копируем шрифт в начало памяти

    srand(time(NULL)); // получение сида для рандомного числа(используется в инструкции Cxkk)
}

bool chip8_load_rom(Chip8 *chip8, const char *filename)
{
    FILE *bin_file = fopen(filename, "rb");   // читаем именно в бинарном виде
    long file_size = get_file_size(bin_file); // получаем размер файла

    if (file_size > 0x1000 - 0x200)
    {
        perror("file: binary file is too large");
        return false;
    }
    fread(chip8->memory + 0x200, sizeof(uint8_t), file_size, bin_file); // копируем бинарник в память

    fclose(bin_file);
    return true;
}

void decode_opcode(uint16_t opcode, decode_nibble *nib_struct)
{
    nib_struct->op = (opcode & 0xF000) >> 12; // получаем 1 бит
    nib_struct->x = (opcode & 0x0F00) >> 8;   // получаем 2 бит
    nib_struct->y = (opcode & 0x00F0) >> 4;   // получаем 3 бит
    nib_struct->n = opcode & 0x000F;          // получаем 4 бит
    nib_struct->kk = opcode & 0x00FF;         // получаем последние 2 бита
    nib_struct->nnn = opcode & 0x0FFF;        // получаем последние 3 бита
}
void chip8_op_draw(Chip8 *chip8, decode_nibble *nib)
{
    chip8->V[15] = 0; // Сбрасываем флаг коллизии
    for (int row = 0; row < nib->n; row++)
    {
        uint8_t sprite_byte = chip8->memory[chip8->I + row];
        // Проверяем бит в байте спрайта (слева направо)
        for (int col = 0; col < 8; col++)
        {
            if ((sprite_byte & (0x80 >> col)))
            {
                uint8_t vx = chip8->V[nib->x];
                uint8_t vy = chip8->V[nib->y];

                uint8_t x = (vx + col) % 64;
                uint8_t y = (vy + row) % 32;

                int index = x + (y * 64);
                if (chip8->gfx[index] == 1) // Если пиксель на экране уже 1, регистрируем коллизию
                {
                    chip8->V[15] = 1;
                }
                // Переключаем пиксель
                chip8->gfx[index] ^= 1;
            }
        }
    }
}
bool chip8_cycle(Chip8 *chip8)
{
    if (!chip8)
        return false;

    uint16_t opcode = (chip8->memory[chip8->pc] << 8) | chip8->memory[chip8->pc + 1];
    chip8->pc += 2;
    decode_nibble nib;
    decode_opcode(opcode, &nib);

    switch (nib.op)
    {
    case 0x0:
        switch (nib.kk)
        {
        case 0xE0: // 00E0 - CLS
            memset(chip8->gfx, 0, sizeof(chip8->gfx));
            break;
        case 0xEE:// 0x00EE - RET
            chip8->sp--;
            chip8->pc = chip8->stack[chip8->sp];
            break;
        default:
            break;
        }
        break;

    case 0x1: // 1nnn - JP addr
        chip8->pc = nib.nnn;
        break;

    case 0x2:                                //  2nnn - CALL addr
        chip8->stack[chip8->sp] = chip8->pc; // Кладем адрес возврата в вершину
        chip8->sp++;                         // Сдвигаем указку стека вверх
        chip8->pc = nib.nnn;                 // Прыгаем на адрес подпрограммы
        break;

    case 0x3: // 3xkk - SE Vx, byte
        if (chip8->V[nib.x] == nib.kk)
        {
            chip8->pc += 2;
        }
        break;
    case 0x4: // 4xkk - SNE Vx, byte
        if (chip8->V[nib.x] != nib.kk)
        {
            chip8->pc += 2;
        }
        break;
    case 0x5: // 5xy0 - SE Vx, Vy

        if (chip8->V[nib.x] == chip8->V[nib.y])
        {
            chip8->pc += 2;
        }
        break;

    case 0x6: // 6xkk - LD Vx, byte
        chip8->V[nib.x] = nib.kk;
        break;

    case 0x7: // 7xkk - ADD Vx, byte
        chip8->V[nib.x] += nib.kk;
        break;

    case 0x8:
        switch (nib.n)
        {
        case 0x0: // 8xy0 - LD Vx, Vy
            chip8->V[nib.x] = chip8->V[nib.y];
            break;

        case 0x1: // 8xy1 - OR Vx, Vy
            chip8->V[nib.x] |= chip8->V[nib.y];
            break;

        case 0x2: // 8xy2 - AND Vx, Vy
            chip8->V[nib.x] &= chip8->V[nib.y];
            break;

        case 0x3: // 8xy3 - XOR Vx, Vy
            chip8->V[nib.x] ^= chip8->V[nib.y];
            break;

        case 0x4: //    8xy4 - ADD Vx, Vy
        {
            uint16_t sum = chip8->V[nib.x] + chip8->V[nib.y];
            chip8->V[nib.x] = sum & 0xFF;
            if (sum > 255)
            {
                chip8->V[15] = 1;
            }
            else
                chip8->V[15] = 0;
        }
        break;

        case 0x5: // 8xy5 - SUB Vx, Vy
            if (chip8->V[nib.x] >= chip8->V[nib.y])
            {
                chip8->V[15] = 1;
            }
            else
                chip8->V[15] = 0;

            chip8->V[nib.x] -= chip8->V[nib.y];
            break;

        case 0x6: // 8xy6 - SHR Vx {, Vy}
        {
            uint8_t ls_bit = (chip8->V[nib.x] & 0x01);

            if (ls_bit)
            {
                chip8->V[15] = 1;
            }
            else
                chip8->V[15] = 0;
            chip8->V[nib.x] >>= 1;
        }
        break;

        case 0x7: // 8xy7 - SUBN Vx, Vy
            if (chip8->V[nib.x] <= chip8->V[nib.y])
            {
                chip8->V[15] = 1;
            }
            else
                chip8->V[15] = 0;

            chip8->V[nib.x] -= chip8->V[nib.y];
            break;

        case 0xE: // 8xyE - SHL Vx {, Vy}
        {
            uint8_t ms_bit = (chip8->V[nib.x] & 0x80);

            if (ms_bit)
            {
                chip8->V[15] = 1;
            }
            else
                chip8->V[15] = 0;
            chip8->V[nib.x] <<= 1;
        }
        break;

        default:
            break;
        }
        break;
    case 0x9: // 9xy0 - SNE Vx, Vy
        if (chip8->V[nib.x] != chip8->V[nib.y])
        {
            chip8->pc += 2;
        }
        break;
    case 0xA: // Annn - LD I, addr
        chip8->I = nib.nnn;
        break;

    case 0xB: // Bnnn - JP V0, addr
        chip8->pc = nib.nnn + chip8->V[0];
        break;

    case 0xC: // Cxkk - RND Vx, byte
    {
        uint8_t rand_byte = rand() % 256;
        chip8->V[nib.x] = rand_byte & nib.kk;
        break;
    }
    case 0xD: // Dxyn - DRW Vx, Vy, nibble
        chip8_op_draw(chip8, &nib);
        break;

    case 0xE:
        switch (nib.kk)
        {
        case 0x9E:
            if (nib.x >= 0 && nib.x < 16) // проверяем правильная ли кнопка
            {
                if (chip8->keypad[chip8->V[nib.x]]) // если кнопка под номером V[x] нажата,
                    chip8->pc += 2;                 // то пропуск
            }
            break;

        case 0xA1:
            if (nib.x >= 0 && nib.x < 16) // проверяем правильная ли кнопка
            {
                if (!chip8->keypad[chip8->V[nib.x]]) // если кнопка под номером V[x] не нажата,
                    chip8->pc += 2;                  // то пропускаем
            }
            break;

        default:
            break;
        }
        break;

    case 0xF:
        switch (nib.kk)
        {
        case 0x07:                                // Fx07 - LD Vx, DT
            chip8->V[nib.x] = chip8->delay_timer; // в V[x] помещаем делей-таймер
            break;

        case 0x0A: // Fx0A - LD Vx, K
            bool key_state = false;
            // ищем нажатые клавиши
            for (int i = 0; i < KEY_COUNT; i++)
            {
                if (chip8->keypad[i]) // если нашли
                {
                    chip8->V[nib.x] = i; // помещаем ее номер в регистр
                    chip8->pc += 2;      // идем на следущею инструкцию
                    key_state = true;    // возводим флаг
                    break;
                }
            }
            if (!key_state) // если не нажата выполняем эту же инструкцию заново
                chip8->pc -= 2;
            break;

        case 0x15:                                // Fx15 - LD DT, Vx
            chip8->delay_timer = chip8->V[nib.x]; // обновляем делей-таймер из регистра
            break;

        case 0x18:                                // Fx18 - LD ST, Vx
            chip8->sound_timer = chip8->V[nib.x]; // обновляем саунд-таймер из регистра
            break;

        case 0x1E: // Fx1E - ADD I, Vx
            chip8->I += chip8->V[nib.x];
            break;

        case 0x29: //  Fx29 - LD F, Vx
            chip8->I = 0x050 + (chip8->V[nib.x] * 5);
            break;

        case 0x33: //  Fx33 - LD B, Vx
            // раскаладываем что лежит в регистре по 100, 10 и единицам
            chip8->memory[chip8->I] = chip8->V[nib.x] / 100;
            chip8->memory[chip8->I + 1] = (chip8->V[nib.x] / 10) % 10;
            chip8->memory[chip8->I + 2] = chip8->V[nib.x] % 10;
            break;

        case 0x55: // Fx55 - LD [I], Vx
            for (int i = 0; i <= nib.x; i++)
            {
                chip8->memory[chip8->I + i] = chip8->V[i];
            }
            break;

        case 0x65: // Fx65 - LD Vx, [I]
            for (int i = 0; i <= nib.x; i++)
            {
                chip8->V[i] = chip8->memory[chip8->I + i];
            }
        default:
            break;
        }
        break;
    default:
        fprintf(stderr, "Unknown Opcode: 0x%04X\n", opcode);
        return false;
        break;
    }
    return true;
}