#ifndef CHIP8_H
#define CHIP8_H

#include <cstdint>
#include <stack>
#include <fstream>
#include <iostream>
#include <cstring>
#include <SDL3/SDL.h>
#include <vector>
#include <utility>
#include <algorithm>

#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32

extern uint8_t display[SCREEN_WIDTH * SCREEN_HEIGHT]; // Declare it globally

struct Registers {
    uint16_t PC = 0x200;
    uint16_t I = 0;
    uint8_t V[16] = {};
};

struct Timers {
    uint8_t delay = 0;
    uint8_t sound = 0;

    void update()
    {
        if (delay)
        {
            delay--;
        }
        if (sound)
        {
            sound--;
            std::cout << "BEEP" << '\n';
        }
    }
};

struct Memory {
    uint8_t data[4096] = {};

    [[nodiscard]] uint8_t read(const uint16_t address) const
    {
        return data[address];
    }

    void write(const uint16_t address, const uint8_t value)
    {
        data[address] = value;
    }
};


struct KeyMap {
    std::vector<std::pair<SDL_Scancode, int>> keys = 
    {{SDL_SCANCODE_X, 0}, {SDL_SCANCODE_1, 0}, {SDL_SCANCODE_2, 0}, {SDL_SCANCODE_3, 0}, {SDL_SCANCODE_Q, 0},
    {SDL_SCANCODE_W, 0}, {SDL_SCANCODE_E, 0}, {SDL_SCANCODE_A, 0}, {SDL_SCANCODE_S, 0}, {SDL_SCANCODE_D, 0},
    {SDL_SCANCODE_Z, 0}, {SDL_SCANCODE_C, 0}, {SDL_SCANCODE_4, 0}, {SDL_SCANCODE_R, 0}, {SDL_SCANCODE_F, 0},
    {SDL_SCANCODE_V, 0}};

    bool contains(SDL_Scancode scancode);
    void set(SDL_Scancode scancode, bool value);
    int get(SDL_Scancode scancode);
};

class Chip8 {
    Registers regs;
    Timers timers;
    Memory memory;
    KeyMap keyMap;
    std::stack<uint16_t> stack;
    

    public:
        SDL_Scancode upPrevious;

        Chip8();
        int writeRom(const char path[]);
        void updateTimers();
        void keyEvent(SDL_Scancode key, bool keyDown);
        uint16_t fetch();
        void decode(int opcode);
};

#endif // CHIP8_H