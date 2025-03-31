#ifndef CHIP8_H
#define CHIP8_H

#include <cstdint>
#include <stack>
#include <fstream>
#include <iostream>
#include <cstring>

#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32

extern uint8_t display[SCREEN_WIDTH * SCREEN_HEIGHT]; // Declare it globally

struct Registers
{
    uint16_t PC = 0x200;
    uint16_t I = 0;
    uint8_t V[16] = {};
};

struct Timers
{
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

struct Memory
{
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


class Chip8 {
    Registers regs;
    Timers timers;
    Memory memory;
    std::stack<uint16_t> stack;

    public:
        int writeRom(const char path[]);
        void updateTimers();
        uint16_t fetch();
        void decode(int opcode);
};

#endif // CHIP8_H