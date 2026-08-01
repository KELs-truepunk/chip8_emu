#include <SDL2/SDL.h>
#include "chip8.h"
#include <stdio.h>

static const SDL_Keycode KEYMAP[16] = {
    SDLK_x, SDLK_1, SDLK_2, SDLK_3,
    SDLK_q, SDLK_w, SDLK_e, SDLK_a,
    SDLK_s, SDLK_d, SDLK_z, SDLK_c,
    SDLK_4, SDLK_r, SDLK_f, SDLK_v};

int window_scale = 15; // коэфицент размера окна, дефолтный 15, иначе берем из argv[2]
const static char *window_title = "CHIP8 Emulator";

// генерация писка
void audio_callback(void *userdata, Uint8 *stream, int len)
{
    static int sample_index = 0;
    int frequency = 440;     // частота звука в Гц (Ля)
    int sample_rate = 44100; // частота дискретизации

    Sint16 *buffer = (Sint16 *)stream;
    int length = len / 2; // 16 битный звук (2 байта на семпл)

    for (int i = 0; i < length; i++)
    {
        // чередуем амплитуду для создания квадратной волны. 3000 — умеренная громкость.
        buffer[i] = ((sample_index++ * frequency / sample_rate) % 2) ? 3000 : -3000;
    }
}

int main(int argc, char **argv)
{
    Chip8 chip8;        // основная структура устройства
    chip8_init(&chip8); // инициализируем структуру

    // парсим аргументы запуска
    if (argc > 3) // много аргументов - выходим
    {
        fprintf(stderr, "Too many arguments! Usage: %s <path_to_bin> [scale] \n", argv[0]);
        return -1;
    }
    if (argc < 2) // мало аргументов - выходим
    {
        fprintf(stderr, "Usage: %s <path_to_rom> [scale]\n", argv[0]);
        printf("Example: %s roms/invaders.ch8 15\n", argv[0]);
        return -1;
    }
    // получаем scale из argv
    if (argc >= 3)
    {
        int user_scale = atoi(argv[2]);
        if (user_scale > 0)
        {
            window_scale = user_scale;
        }
    }
    const char *filename = argv[1];   // файл с бинарником
    chip8_load_rom(&chip8, filename); // грузим его в ОЗУ
    // Инициалзация SDL2
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
    {
        fprintf(stderr, "SDL Init Error: %s\n", SDL_GetError());
        return -1;
    }

    // настройка аудиоустройства
    SDL_AudioSpec desired_spec;
    desired_spec.freq = 44100;
    desired_spec.format = AUDIO_S16SYS;
    desired_spec.channels = 1;
    desired_spec.samples = 2048;
    desired_spec.callback = audio_callback;
    desired_spec.userdata = NULL;

    SDL_AudioDeviceID audio_device = SDL_OpenAudioDevice(NULL, 0, &desired_spec, NULL, 0);
    if (audio_device == 0)
    {
        fprintf(stderr, "SDL Audio Error: %s\n", SDL_GetError());
        // Не выходим из программы, если звуковая карта занята — пусть игра идет без звука
    }
    // создаем окно с положением в центре, с заголовком, без флагов, разрешение 64 на 32
    SDL_Window *window = SDL_CreateWindow(
        window_title,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH * window_scale,
        SCREEN_HEIGHT * window_scale, 0);

    bool running = true;
    SDL_Event event;

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    if (!renderer)
    {
        fprintf(stderr, "Renderer Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    while (running)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT) // если выходим
            {
                running = false; // прекращаем цикл
            }
            else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) // если клавиша нажата или отжата
            {
                for (int i = 0; i < 16; i++)
                {
                    if (event.key.keysym.sym == KEYMAP[i]) // какая именно нажата?
                    {
                        // Если KEYDOWN — пишем 1, если KEYUP — пишем 0
                        chip8.keypad[i] = (event.type == SDL_KEYDOWN) ? 1 : 0;
                        break;
                    }
                }
            }
        }
        // 10 инструкций за кадр
        for (int i = 0; i < 10; i++)
        {
            chip8_cycle(&chip8);
        }
        if (chip8.delay_timer > 0)
        {
            chip8.delay_timer--;
        }
        if (chip8.sound_timer > 0)
        {
            chip8.sound_timer--;
        }

        // логика звукового таймера CHIP-8
        if (chip8.sound_timer > 0)
        {
            if (audio_device > 0)
            {
                SDL_PauseAudioDevice(audio_device, 0); // вкл звук
            }
            chip8.sound_timer--;
        }
        else
        {
            if (audio_device > 0)
            {
                SDL_PauseAudioDevice(audio_device, 1); // выкл звук
            }
        }
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);       // ставим черный
        SDL_RenderClear(renderer);                            // чистим экран
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // вкл белый
        for (int y = 0; y < SCREEN_HEIGHT; y++)
        {
            for (int x = 0; x < SCREEN_WIDTH; x++)
            {

                // проверяем пиксель в массиве
                if (chip8.gfx[y * SCREEN_WIDTH + x])
                {
                    SDL_Rect rect = {
                        x * window_scale,
                        y * window_scale,
                        window_scale,
                        window_scale};
                    SDL_RenderFillRect(renderer, &rect);
                }
            }
        }
        // вывод картинки
        SDL_RenderPresent(renderer);
        SDL_Delay(16); //~60 FPS
    }
    //очистка
    if (audio_device > 0)
    {
        SDL_CloseAudioDevice(audio_device);
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
