#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdint>
#include <SDL3/SDL.h>

#include "chip8.hpp"

constexpr int fontAddress = 0x50;

static uint8_t font[] = {
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

Chip8::Chip8() :rnd(std::chrono::steady_clock::now().time_since_epoch().count()) 
{
    memcpy(&memory[fontAddress], font, sizeof(font));
}

void Chip8::loadROM(const char* filename) 
{
    std::ifstream file;
    file.open(filename, std::ios::binary | std::ios::ate);
    if(!file.is_open()) {
        std::cout << "could not open file";
        return;
    }

    int length = file.tellg();

    //if file is too big
    if(length > 0xE00) {
        std::cout << "program too big\n";
        file.close();
        return;
    }
    // else if(length > 0xE00) { // if the program is bigger than the 3.5kb block of memory but fits in 4kb
    //     startAddress = 0;
    //     std::cout << "extended memory mode activated\n";
    // } 
    

    file.seekg(file.beg);
    file.read((char*)&memory[startAddress], length);

    file.close();
    std::cout << "file loaded successfully\n";
    PC = startAddress;
}

void Chip8::processInput() 
{
    const bool* keystate = SDL_GetKeyboardState(NULL);
    keypad[0] = keystate[SDL_SCANCODE_X];
    keypad[1] = keystate[SDL_SCANCODE_1];
    keypad[2] = keystate[SDL_SCANCODE_2];
    keypad[3] = keystate[SDL_SCANCODE_3];
    keypad[4] = keystate[SDL_SCANCODE_Q];
    keypad[5] = keystate[SDL_SCANCODE_W];
    keypad[6] = keystate[SDL_SCANCODE_E];
    keypad[7] = keystate[SDL_SCANCODE_A];
    keypad[8] = keystate[SDL_SCANCODE_S];
    keypad[9] = keystate[SDL_SCANCODE_D];
    keypad[10] = keystate[SDL_SCANCODE_Z];
    keypad[11] = keystate[SDL_SCANCODE_C];
    keypad[12] = keystate[SDL_SCANCODE_4];
    keypad[13] = keystate[SDL_SCANCODE_R];
    keypad[14] = keystate[SDL_SCANCODE_F];
    keypad[15] = keystate[SDL_SCANCODE_V];
}

void Chip8::execIns()
{
    uint16_t opcode = memory[PC] << 8 | memory[PC+1];
    PC += 2;

    if(soundTimer > 0)
        soundTimer--;

    if(delayTimer > 0)
        delayTimer--;

    // 0xDXYN, 0xDNNN, 0xDXKK
    uint8_t d = (opcode & 0xF000) >> 12;
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;
    uint8_t n = opcode & 0x000F;
    uint8_t kk = opcode & 0x00FF;
    uint16_t nnn = opcode & 0x0FFF;

    // uint8_t& xReg = registers[x];
    // uint8_t& yReg = registers[y];

    switch(d) {
        case 0x0:
            if(opcode == CLS) // clear videoRAM
                memset(videoRam, 0, sizeof(videoRam));
            else if(opcode == RET) // return
                PC = stack[SP--];
            break;

        case JMP: // has to be jmp
            PC = nnn;
            break;

        case CALL:
            stack[++SP] = PC;
            PC = nnn;
            break;
        case SEimm:
            PC = (registers[x] == kk) ? PC + 2 : PC; 
            break;
        case SNEimm:
            PC = (registers[x] != kk) ? PC + 2 : PC; 
            break;

        case SE:
            PC = (registers[x] == registers[y]) ? PC + 2 : PC;
            break;
        case LD:
            registers[x] = kk;
            break;

        case ADD:
            registers[x] += kk;
            break;

        case 8:
            if(n == 0) 
                registers[x] = registers[y];
            else if(n == 1)
                registers[x] |= registers[y];
            else if(n == 2)
                registers[x] &= registers[y];
            else if(n == 3)
                registers[x] ^= registers[y];
            else if(n == 4) {
                uint16_t sum = registers[x] + registers[y];
                registers[0xF] = sum > 255;
                registers[x] = sum & 0xFF;
            }
            else if(n == 5) {
                registers[0xF] = registers[x] > registers[y];
                registers[x] -= registers[y];
            }
            else if(n == 6) {
                registers[0xF] = registers[x] & 0x1;
                registers[x] >>= 1;
            }
            else if(n == 7) {
                registers[0xF] = registers[y] > registers[x];
                registers[x] = registers[y] - registers[x];
            }
            else if(n == 0xE) {
                registers[0xF] = registers[x] >> 7;
                registers[x] <<= 1;
            }
            break;

        case 0x9:
            PC = (registers[x] != registers[y]) ? PC + 2 : PC;
            break;

        case 0xA:
            I = nnn;
            break;

        case 0xB:
            PC = nnn + registers[0];
            break;

        case 0xC:
            registers[x] = std::uniform_int_distribution<uint8_t>()(rnd) & kk;
            break;

        case 0xD:
            { // scope for variables
            const uint8_t height = n;
            const uint8_t xPos = registers[x] % 64;
            const uint8_t yPos = registers[y] % 32;

            registers[0xF] = 0;

            for(int row = 0; row < height; row++) {
                uint8_t spriteByte = memory[I + row];

                for(int byte = 0; byte < 8; byte ++) {
                    
                    uint8_t spritePixel = spriteByte & (0x80 >> byte);
                    uint8_t* screenPixel = &videoRam[(yPos + row) * 64 + xPos + byte];

                    if(spritePixel) {
                        registers[0xF] = screenPixel[0] == 255;
                        screenPixel[0] ^= 255;
                    }
                }
            }
            }
            break;

        case 0xE:
            if(kk == 0x9E)
                PC = (keypad[registers[x]]) ? PC + 2 : PC;
            else if(kk == 0xA1)
                PC = (!keypad[registers[x]]) ? PC + 2 : PC;
            break;

        case 0xF:
            if(kk == 0x07)
                registers[x] = delayTimer;
            else if(kk == 0x0A) { //wait for keypress and loops
                bool keyPressed = false;
                for(int i = 0; i < 16; i++) {
                    if(keypad[i]) {
                        registers[x] = keypad[i];
                        keyPressed = true;
                        break;
                    }
                }
                if(!keyPressed)
                    PC -= 2;
            }
            else if(kk == 0x15) 
                delayTimer = registers[x];
            else if(kk == 0x18) 
                soundTimer = registers[x];
            else if(kk == 0x1E) 
                I += registers[x];
            else if(kk == 0x29)
                I = fontAddress + registers[x] * 5;
            else if(kk == 0x33) {
                memory[I] = registers[x] / 100;
                memory[I+1] = (registers[x] / 10) % 10;
                memory[I+2] = registers[x] % 10;
            }
            else if(kk == 0x55) {
                for(int i = 0; i <= x;i++)
                    memory[I+i] = registers[i];
            }
            else if(kk == 0x65) {
                for(int i = 0; i <= x;i++)
                    registers[i] = memory[I+i];
            }
            break;
        default:
            std::cout << "Unimplemented Instruction: " << std::hex << PC << "\n";
            break;
    }

}