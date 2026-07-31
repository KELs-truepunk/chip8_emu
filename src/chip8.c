#include "chip8.h"
#include <string.h>
void chip8_init(Chip8* chip8){
    chip8->pc = 0x200; //код программы начинается именно с 0x200
    chip8->I = 0;
    chip8->sp = 0;
    chip8->delay_timer = 0;
    chip8->sound_timer = 0;
    memset(chip8->memory, MEMORY_SIZE, 0);

}
void chip8_symbols_init(uint8_t* memory){
    for(int i = 0; i < 80; i++){
        
    }
}
bool chip8_load_rom(Chip8* chip8, const char *filename){


}