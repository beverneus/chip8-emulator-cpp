// Copyright 2025 Lars De Volder
#include <math.h>
#include <SDL2/SDL.h>
#include <cstdint>
#include <iostream>
#include <stack>

#define GET_X(opcode) ((opcode & 0x0F00) >> 8)
#define GET_Y(opcode) ((opcode & 0x00F0) >> 4)
#define GET_N(opcode) (opcode & 0x000F)
#define GET_NN(opcode) (opcode & 0x00FF)
#define GET_NNN(opcode) (opcode & 0x0FFF)

#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32
#define SCALE_FACTOR 8
#define WINDOW_WIDTH (SCREEN_WIDTH * SCALE_FACTOR)
#define WINDOW_HEIGHT (SCREEN_HEIGHT * SCALE_FACTOR)

SDL_Window *window = nullptr;
SDL_Surface *winSurface = nullptr;
SDL_Surface *chip8Surface = nullptr;
uint8_t display[SCREEN_WIDTH * SCREEN_HEIGHT] = {};

struct Registers {
    uint PC = 0x200;
    uint16_t I = 0;
    uint8_t V[16] = {};
    uint8_t delayTimer = 0;
    uint8_t soundTimer = 0;
};

struct Memory {
    uint8_t data[4096] = {};

    [[nodiscard]] uint8_t read(const uint16_t address) const {
        return data[address];
    }

    void write(const uint16_t address, const uint8_t value) {
        data[address] = value;
    }
};

class Chip8 {
    Registers regs;
    Memory memory;
    std::stack<int> stack;

    uint16_t opcode = 0x0;

        public:
            void fetch() {
                uint* PC = &this->regs.PC;
                const uint8_t a = this->memory.read(*PC);
                const uint8_t b = this->memory.read(*PC + 1);
                *PC += 2;
                opcode = a << 8 | b;
            }

            void decode() {
            }
};

void draw() {
    SDL_FillRect(chip8Surface, nullptr,
        SDL_MapRGB(chip8Surface->format, 0, 0, 0));
}

bool loop();

int main() {
    SDL_Init(SDL_INIT_EVERYTHING);

    window = SDL_CreateWindow("Chip-8",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    winSurface = SDL_GetWindowSurface(window);

    chip8Surface = SDL_CreateRGBSurface(0,
        SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0, 0, 0, 0);

    while ( loop() ) {
    }

    Chip8 chip;
    return 0;
}

bool loop() {
    const Uint64 start = SDL_GetPerformanceCounter();
    // Event loop
    SDL_Event evt;
    while (SDL_PollEvent(&evt) != 0) {
        switch (evt.type) {
            case SDL_QUIT:
                return false;
            default:
                break;
        }
    }

    const Uint64 end = SDL_GetPerformanceCounter();
    const float elapsedMS = (end - start) /
        static_cast<float>(SDL_GetPerformanceFrequency()) * 1000.0f;

    SDL_Delay(fmax(0.0f, floor(16.666f - elapsedMS)));

    SDL_UpdateWindowSurface(window);

    return true;
}
