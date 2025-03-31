#include "chip8.hpp"

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


int Chip8::writeRom(const char path[]) {
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
    }
}
