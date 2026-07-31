#include "chip8.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
//размер файла в байтах
long get_file_size(FILE* file){
    if(file){
        fseek(file, 0, SEEK_END); //переходим в конец файла
        long file_size = ftell(file); //узнаем, где находится указатель, чтобы понять какого размера файл
        rewind(file);
        return file_size;
    }else {
        perror("file: file is empty");
        return -1;
    }
}

void fontset_to_mem(uint8_t* memory){
    for(int i = 0; i < 80; i++){
        memory[0x050 + i] = fontset[i]; 
    }
}

void chip8_init(Chip8* chip8){
    chip8->pc = 0x200; //код программы начинается именно с 0x200
    chip8->I = 0;
    chip8->sp = 0;
    chip8->delay_timer = 0;
    chip8->sound_timer = 0;
    
    memset(chip8->memory, 0, MEMORY_SIZE);
    fontset_to_mem(chip8->memory);

    memset(chip8->V, 0, REGISTER_COUNT); 
}

bool chip8_load_rom(Chip8* chip8, const char *filename){
    FILE* bin_file = fopen(filename, "rb");
    long file_size = get_file_size(bin_file);

    if (file_size > 0x1000 - 0x200){ 
        perror("file: binary file is too large");
        return false;
    }
    fread(chip8->memory + 0x200, sizeof(uint8_t), file_size, bin_file); //копируем бинарник в память
    
    fclose(bin_file);
    return true;

}