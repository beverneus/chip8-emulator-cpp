// Copyright 2025 Lars De Volder
#include <math.h>
#include <SDL3/SDL.h>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <stack>
#include <iostream>
#include <fstream>

#define GET_X(opcode) ((opcode & 0x0F00) >> 8)
#define GET_Y(opcode) ((opcode & 0x00F0) >> 4)
#define GET_N(opcode) (opcode & 0x000F)
#define GET_NN(opcode) (opcode & 0x00FF)
#define GET_NNN(opcode) (opcode & 0x0FFF)

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
    const SDL_PixelFormatDetails *format = SDL_GetPixelFormatDetails(chip8Surface->format);

    const Uint32 BLACK = SDL_MapRGB(format, nullptr, 0, 0, 0);
    const Uint32 WHITE = SDL_MapRGB(format, nullptr, 255, 255, 255);

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

struct Registers {
    uint16_t PC = 0x200;
    uint16_t I = 0;
    uint8_t V[16] = {};
};

struct Timers {
    uint8_t delay = 0;
    uint8_t sound = 0;

    void update() {
        if (delay) {
            delay -= 1;
        }
        if (sound) {
            sound -= 1;
            std::cout << "BEEP" << '\n';
        }
    }

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
        Timers timers;
        Memory memory;
        std::stack<uint16_t> stack;

        uint16_t opcode = 0x0;

    public:
        int writeRom(const char path[]) {
            std::ifstream rom(path, std::ios::binary);
            if (rom.fail()) {
                std::cout << "Failed to read rom" << std::endl;
                return 1;
            }
            uint8_t x;
            uint16_t i = 0;
            while (!rom.eof()) {
                rom.read(reinterpret_cast<char*>(&x), 1);
                memory.data[0x200 + i] = x;
                i++;
            }
            rom.close();
            return 0;
        }

        void updateTimers() {
            timers.update();
        }

        void fetch() {
            const uint8_t a = memory.read(regs.PC);
            const uint8_t b = memory.read(regs.PC + 1);
            regs.PC += 2;
            opcode = a << 8 | b;
        }

        void decode() {
            uint8_t category = (opcode & 0xF000) >> 12;
            switch (category) {
                case 0x0:
                    switch (GET_NN(opcode)) {
                        case 0xE0:
                            std::memset(display, 0, sizeof(display));
                            break;
                        case 0xEE:
                            regs.PC = stack.top();
                            stack.pop();
                            break;
                    }
                    break;
                case 0x1:
                    regs.PC = GET_NNN(opcode);
                    break;
                case 0x2:
                    stack.push(regs.PC);
                    regs.PC = GET_NNN(opcode);
                    break;
                case 0x6:
                    regs.V[GET_X(opcode)] = GET_NN(opcode);
                    break;
                case 0x7:
                    regs.V[GET_X(opcode)] += GET_NN(opcode);
                    break;
                case 0xA:
                    regs.I = GET_NNN(opcode);
                    break;
                case 0xD:
                    uint8_t sprite_x = regs.V[GET_X(opcode)] % SCREEN_WIDTH;
                    uint8_t sprite_y = regs.V[GET_Y(opcode)] % SCREEN_HEIGHT;
                    uint8_t* sprite = &memory.data[regs.I];
                    uint8_t* VF = &regs.V[0xF];
                    *VF = 0;
                    for (int y = 0; y < GET_N(opcode); y++) {
                        for (int x = 0; x < 8; x++) {
                            if (*(sprite + y) & (0b1 << (7-x))) { // Go over all 8 bits in the byte, from left to right
                                int draw_x = sprite_x + x;
                                int draw_y = sprite_y + y;
                                if (draw_x < SCREEN_WIDTH && draw_y < SCREEN_HEIGHT) {
                                    if (display[draw_y * SCREEN_WIDTH + draw_x]) {
                                        *VF = 1;
                                        display[draw_y * SCREEN_WIDTH + draw_x] = 0;
                                    } else {
                                        display[draw_y * SCREEN_WIDTH + draw_x] = 1;
                                    }
                                }
                            }
                        }
                    }
                    break;
            }
        }
};

bool loop();
Chip8 chip;

int main(int, char *argv[]) {
    //ROM
    chip.writeRom(argv[1]);

    //DISPLAY
    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow("Chip-8", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_INPUT_FOCUS);
    winSurface = SDL_GetWindowSurface(window);
    chip8Surface = SDL_CreateSurface(SCREEN_WIDTH, SCREEN_HEIGHT, SDL_PIXELFORMAT_ARGB32);

    while ( loop() ) {
    }

    return 0;
}

bool loop() {
    const Uint64 start = SDL_GetPerformanceCounter();
    // Event loop
    SDL_Event evt;
    while (SDL_PollEvent(&evt) != 0) {
        switch (evt.type) {
            case SDL_EVENT_QUIT:
                return false;
            default:
                break;
        }
    }
    for (int i=0; i<ceil(IPS/60); i++) {
        chip.fetch();
        chip.decode();
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
