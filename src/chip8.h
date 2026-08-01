#pragma once 

#include <stdint.h>
#include <stdbool.h>

#define MEMORY_SIZE 4096 
#define REGISTER_COUNT 16
#define STACK_LEVELS 16
#define KEY_COUNT 16

#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32

typedef struct {
    uint8_t memory[MEMORY_SIZE];         // 4 КБ RAM
    uint8_t V[REGISTER_COUNT];           // Регистры V0-VF
    uint16_t I;                          // Индексный регистр
    uint16_t pc;                         // Счетчик команд (Program Counter)
    
    uint16_t stack[STACK_LEVELS];        // Стек для CALL/RET
    uint8_t sp;                          // Указатель стека (Stack Pointer)
    
    uint8_t delay_timer;                 // Таймер задержки
    uint8_t sound_timer;                 // Звуковой таймер
    
    uint8_t keypad[KEY_COUNT];           // Состояние 16 клавиш (0 или 1)
    uint32_t gfx[SCREEN_WIDTH * SCREEN_HEIGHT]; // Пиксели экрана (ARGB)
} Chip8;

typedef struct decode_nibble
{
    uint8_t op;     //1 бит   
    uint8_t x;      //2 бит
    uint8_t y;      //3 бит
    uint8_t n;      //4 бит
    uint8_t kk;     //последние 2 бита
    uint16_t nnn;   // последние3 бита 
} decode_nibble;        

//Шрифт
const uint8_t fontset[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, //0
    0x20, 0x60, 0x20, 0x20, 0x70, //1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
    0x90, 0x90, 0xF0, 0x10, 0x10, //4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
    0xF0, 0x10, 0x20, 0x40, 0x40, //7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
    0xF0, 0x90, 0xF0, 0x90, 0x90, //A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, //Bё
    0xF0, 0x80, 0x80, 0x80, 0xF0, //C
    0xE0, 0x90, 0x90, 0x90, 0xE0, //D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
    0xF0, 0x80, 0xF0, 0x80, 0x80  //F 
};
void chip8_init(Chip8* chip8); //Инициализация
bool chip8_load_rom(Chip8* chip8, const char *filename); //Загрузка ROM в RAM
bool chip8_cycle(Chip8* chip8); //Декодинг бинарника и выполнение 