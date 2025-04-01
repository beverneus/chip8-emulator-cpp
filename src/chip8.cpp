#include "chip8.hpp"
#include "audio.hpp"

#define GET_X(opcode) ((opcode & 0x0F00) >> 8)
#define GET_Y(opcode) ((opcode & 0x00F0) >> 4)
#define GET_N(opcode) (opcode & 0x000F)
#define GET_NN(opcode) (opcode & 0x00FF)
#define GET_NNN(opcode) (opcode & 0x0FFF)

#define VX (regs.V[GET_X(opcode)])
#define VY (regs.V[GET_Y(opcode)])
#define VF (regs.V[0xF])

#undef RAND_MAX
#define RAND_MAX UINT8_MAX

bool KeyMap::contains(SDL_Scancode scancode) {
    for (const auto& pair : keys) {
        if (pair.first == scancode) return true;
    }
    return false;
}

void KeyMap::set(SDL_Scancode scancode, bool value) {
    auto it = std::find_if(keys.begin(), keys.end(), 
    [scancode](const std::pair<SDL_Scancode, int>& p) { return p.first == scancode; });
    if (it != keys.end()) {
        it->second = value;
    } 
}

int KeyMap::get(SDL_Scancode scancode) {
    auto it = std::find_if(keys.begin(), keys.end(), 
    [scancode](const std::pair<SDL_Scancode, int>& p) { return p.first == scancode; });
    if (it != keys.end()) {
        return std::distance(keys.begin(), it);
    } else {
        return -1;
    }
}

Chip8::Chip8() : upPrevious(SDL_SCANCODE_UNKNOWN) {
    // FONT
    uint8_t font[5*16] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };
    int i = 0;
    for (uint8_t x : font) {
        memory.write(0x50 + i, x);
        i++;
    }
}

int Chip8::loadRom(const char path[]) {
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

void Chip8::updateTimers() {
    timers.update();
}

void Chip8::keyEvent(SDL_Scancode key, bool keyDown) {
    if (!keyMap.contains(key))
        return;
    keyMap.set(key, keyDown);
}

uint16_t Chip8::fetch() {
    const uint8_t a = memory.read(regs.PC);
    const uint8_t b = memory.read(regs.PC + 1);
    regs.PC += 2;
    uint16_t opcode = a << 8 | b;
    return opcode;
}

void Chip8::decode(int opcode) {
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
        case 0x3:
            if (VX == GET_NN(opcode)) {
                regs.PC += 2;
            }
            break;
        case 0x4:
            if (VX != GET_NN(opcode)) {
                regs.PC += 2;
            }
            break;
        case 0x5:
            if (VX == VY) {
                regs.PC += 2;
            }
            break;
        case 0x6:
            VX = GET_NN(opcode);
            break;
        case 0x7:
            VX += GET_NN(opcode);
            break;
        case 0x8:
            switch (GET_N(opcode)) {
                case 0x0:
                    VX = VY;
                    break;
                case 0x1:
                    VX |= VY;
                    break;
                case 0x2:
                    VX &= VY;
                    break;
                case 0x3:
                    VX ^= VY;
                    break;
                case 0x4:
                    if (VX > UINT8_MAX - VY) {
                        VF = 1;
                    } else {
                        VF = 0;
                    }
                    VX += VY;
                    break;
                case 0x5:
                    if (VX > VY) {
                        VF = 1;
                    } else {
                        VF = 0;
                    }
                    VX -= VY;
                    break;
                case 0x6:
                    VF = VX & 0b1;
                    VX = VX >> 1;
                    break;
                case 0x7:
                    if (VY > VX) {
                        VF = 1;
                    } else {
                        VF = 0;
                    }
                    VX = VY - VX;
                    break;
                case 0xE:
                    VF = VX & 0b10000000;
                    VX = VX << 1;
                    break;
            }
            break;
        case 0x9:
            if (VX != VY) {
                regs.PC += 2;
            }
            break;
        case 0xA:
            regs.I = GET_NNN(opcode);
            break;
        case 0xB:
            regs.PC = GET_NNN(opcode) + VX;
            break;
        case 0xC:
            VX = std::rand() & GET_NN(opcode);
            break;
        case 0xD:
            {
                uint8_t sprite_x = VX % SCREEN_WIDTH;
                uint8_t sprite_y = VY % SCREEN_HEIGHT;
                uint8_t* sprite = &memory.data[regs.I];
                VF = 0;
                for (int y = 0; y < GET_N(opcode); y++) {
                    for (int x = 0; x < 8; x++) {
                        if (*(sprite + y) & (0b1 << (7-x))) { // Go over all 8 bits in the byte, from left to right
                            int draw_x = sprite_x + x;
                            int draw_y = sprite_y + y;
                            if (draw_x < SCREEN_WIDTH && draw_y < SCREEN_HEIGHT) {
                                if (display[draw_y * SCREEN_WIDTH + draw_x]) {
                                    VF = 1;
                                    display[draw_y * SCREEN_WIDTH + draw_x] = 0;
                                } else {
                                    display[draw_y * SCREEN_WIDTH + draw_x] = 1;
                                }
                            }
                        }
                    }
                }
            }
            break;
        case 0xE:
            switch (GET_NN(opcode)) {
                case 0x9E:
                    if (keyMap.keys[VX].second) 
                        regs.PC += 2;
                    break;
                case 0xA1:
                    if (!keyMap.keys[VX].second)
                        regs.PC += 2;
                    break;
            }
        break;
        case 0xF:
            switch (GET_NN(opcode)) {
                case 0x07:
                    VX = timers.delay;
                    break;
                case 0x0A:
                    if (upPrevious == SDL_SCANCODE_UNKNOWN or !keyMap.contains(upPrevious)) {
                        regs.PC -= 2;
                        break;
                    }
                    VX = keyMap.get(upPrevious);
                    break;
                case 0x15:
                    timers.delay = VX;
                    break;
                case 0x18:
                    timers.sound = VX;
                    break;
                case 0x1E:
                    regs.I += VX;
                    if (regs.I >= 0x1000)
                        VF = 1;
                    break;
                case 0x29:{
                    uint8_t character = VX & 0x0F;
                    regs.I = 0x50 + 5*character;
                    }
                    break;
                case 0x33:
                    {
                        memory.write(regs.I, VX / 100);
                        memory.write(regs.I + 1, (VX % 100) / 10);
                        memory.write(regs.I + 2, VX % 10);
                    }
                    break;
                case 0x55:
                    for (int i = 0; i <= GET_X(opcode); i++) {
                        memory.write(regs.I + i, regs.V[i]);
                    }
                    break;
                case 0x65:
                    for (int i = 0; i <= GET_X(opcode); i++) {
                        regs.V[i] = memory.read(regs.I + i);
                    }
                    break;
            }
    }
}
