#pragma once
#include <iostream>
#include <fstream>



struct Chip8 {
    uint8_t memory[4096];
    uint8_t videoRam[64*32];

    uint8_t registers[16];
    uint8_t soundTimer, delayTimer, SP;
    uint16_t PC, I;
    uint16_t stack[10];
    
    uint16_t startAddress = 0x200;

    void loadROM(const char* filename);
    void execIns();

};