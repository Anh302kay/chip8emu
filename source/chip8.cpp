#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdint>

#include "chip8.hpp"

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

void Chip8::loadROM(const char* filename) {
    std::ifstream file;
    file.open(filename, std::ios::binary | std::ios::ate);
    if(!file.is_open()) {
        std::cout << "could not open file";
        return;
    }

    int length = file.tellg();

    //if file is too big
    if(length > 0x1000) {
        std::cout << "program too big\n";
    }
    else if(length > 0xE00) { // if the program is bigger than the 3.5kb block of memory but fits in 4kb
        startAddress = 0;
        std::cout << "extended memory mode activated\n";
    } 
    

    file.seekg(file.beg);
    file.read((char*)&memory[startAddress], length);

    file.close();
    std::cout << "file loaded successfully\n";
    PC = startAddress;
}

void Chip8::execIns()
{
    uint16_t opcode = memory[PC] << 8 | memory[PC+1];


    // 0xDXYN, 0xDNNN, 0xDXKK
    uint8_t d = (opcode & 0xF000) >> 12;
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;
    uint8_t n = opcode & 0x000F;
    uint8_t kk = opcode & 0x00FF;
    uint16_t nnn = opcode & 0x0FFF;

    switch(d) {
        case 0x0:
            if(opcode == 0x00E0) // clear videoRAM
                memset(videoRam, 0, sizeof(videoRam));
            else if(opcode == 0x00EE) // return
                PC = stack[SP--];
            break;

        case 0x1: // has to be jmp
            PC = nnn;
            break;
        
        case 0x6:
            registers[x] = kk;
            break;

        case 0x7:
            registers[x] += kk;
            break;

        case 0xA:
            I = nnn;
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
                        if(screenPixel[0] == 255)
                            registers[0xF] = 1;
                        screenPixel[0] ^= 255;
                    }
                }
            }
            }
            break;

        default:
            break;
    }



    PC += 2;
}