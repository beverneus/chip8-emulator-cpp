// Copyright 2025 Lars De Volder
#include <math.h>
#include <SDL3/SDL.h>
#include <cstdint>
#include <ctime>
#include <iostream>
#include <fstream>

#include "chip8.hpp"

#define IPS 700 // Number of CHIP8 instructions executed every second (not exact, exact number is 60*ceil(IPS/60))
#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32
#define SCALE_FACTOR 16
#define WINDOW_WIDTH (SCREEN_WIDTH * SCALE_FACTOR)
#define WINDOW_HEIGHT (SCREEN_HEIGHT * SCALE_FACTOR)

SDL_Window *window = nullptr;
SDL_Surface *winSurface = nullptr;
SDL_Surface *chip8Surface = nullptr;
uint8_t display[SCREEN_WIDTH * SCREEN_HEIGHT] = {};


void draw() {
    const Uint32 BLACK = SDL_MapSurfaceRGB(chip8Surface, 0, 0, 0);
    const Uint32 WHITE = SDL_MapSurfaceRGB(chip8Surface, 255, 255, 255);

    SDL_FillSurfaceRect(chip8Surface, nullptr, BLACK);
    
    for (int y = 0; y < SCREEN_HEIGHT; y++)
    {
        for (int x = 0; x < SCREEN_WIDTH; x++)
        {
            if (display[y * SCREEN_WIDTH + x])
            {
                SDL_Rect pixel = {x, y, 1, 1};
                SDL_FillSurfaceRect(chip8Surface, &pixel, WHITE);
            }
        }
    }
    SDL_BlitSurfaceScaled(chip8Surface, nullptr, winSurface, nullptr, SDL_SCALEMODE_NEAREST);
}

bool loop();
Chip8 chip;

int main(int, char *argv[]) {
    SDL_Init(SDL_INIT_VIDEO);

    //ROM
    chip.loadRom(argv[1]);

    //DISPLAY
    window = SDL_CreateWindow("Chip-8", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_INPUT_FOCUS);
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    winSurface = SDL_GetWindowSurface(window);
    chip8Surface = SDL_CreateSurface(SCREEN_WIDTH, SCREEN_HEIGHT, SDL_PIXELFORMAT_ARGB32);

    // seed random number generator
    std::srand(std::time(nullptr));

    while ( loop() ) {
    }

    return 0;
}

bool loop() {
    const Uint64 start = SDL_GetPerformanceCounter();
    chip.upPrevious = SDL_SCANCODE_UNKNOWN;
    // Event loop
    SDL_Event evt;
    while (SDL_PollEvent(&evt) != 0) {
        switch (evt.type) {
            case SDL_EVENT_QUIT:
                return false;
            case SDL_EVENT_KEY_DOWN:
                chip.keyEvent(evt.key.scancode, true);
                break;
            case SDL_EVENT_KEY_UP:
                chip.keyEvent(evt.key.scancode, false);
                chip.upPrevious = evt.key.scancode;
                break;
            default:
                break;
        }
    }
    for (int i=0; i<ceil(IPS/60); i++) {
        uint16_t opcode = chip.fetch();
        chip.decode(opcode);
    }

    chip.updateTimers();

    draw();

    const Uint64 end = SDL_GetPerformanceCounter();
    const float elapsedMS = (end - start) /
        static_cast<float>(SDL_GetPerformanceFrequency()) * 1000.0f;

    SDL_Delay(fmax(0.0f, floor(16.666f - elapsedMS)));

    SDL_UpdateWindowSurface(window);

    return true;
}
