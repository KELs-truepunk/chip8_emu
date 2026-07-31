cmake_minimum_required(VERSION 3.12)

project(chip8_emu C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(SDL2 REQUIRED)

add_executable(chip8_emu src/main.c
                        src/chip8.h
                        src/chip8.c)

# Подключение заголовочных файлов и линковка библиотеки SDL2 к вашему приложению
target_link_libraries(chip8_emu PRIVATE SDL2::SDL2)
