#include <SDL2/SDL.h>
#include "chip8.h"
#include <stdio.h>

int window_scale = 15; //коэфицент размера окна, дефолтный 15, иначе берем из argv[2] 
const static char *window_title = "CHIP8 Emulator";

int main(int argc, char **argv)
{
    Chip8 chip8; //основная структура устройства
    chip8_init(&chip8);//инициализируем структуру

    // парсим аргументы запуска
    if (argc > 2) //много аргументов - выходим
    {
        fprintf(stderr, "Too many arguments! Usage: %s <path_to_bin> [scale] \n", argv[0]);
    }
    if (argc < 2)// мало аргументов - выходим
    {
        fprintf(stderr, "Usage: %s <path_to_rom> [scale]\n", argv[0]);
        printf("Example: %s roms/invaders.ch8 15\n", argv[0]);
        return -1;
    }
    //получаем scale из argv
    if (argc >= 3) {
        int user_scale = atoi(argv[2]);
        if (user_scale > 0) {
            window_scale = user_scale;
        }
    }
    const char* filename = argv[1]; //файл с бинарником
    
    // Инициалзация SDL2
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        fprintf(stderr, "SDL Init Error: %s\n", SDL_GetError());
        return -1;
    }
    // создаем окно с положением в центре, с заголовком, без флагов, разрешение 64 на 32
    SDL_Window *window = SDL_CreateWindow(
        window_title,
        SDL_WINDOWPOS_CENTERED * window_scale,
        SDL_WINDOWPOS_CENTERED * window_scale,
        SCREEN_WIDTH, SCREEN_HEIGHT, 0);
}   