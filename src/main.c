#include <SDL2/SDL.h>
#include "chip8.h"
#include <stdio.h>

const static char* window_title = "CHIP8 Emulator";

int main()
{

    // Инициалзация SDL2
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        fprintf(stderr, "SDL Init Error: %s\n", SDL_GetError());
        return -1;
    }
    //создаем окно с положением в центре, с заголовком, без флагов, разрешение 64 на 32
    SDL_Window* window = SDL_CreateWindow(
        window_title, 
        SDL_WINDOWPOS_CENTERED, 
        SDL_WINDOWPOS_CENTERED, 
        SCREEN_WIDTH, SCREEN_HEIGHT, 0
    );

}