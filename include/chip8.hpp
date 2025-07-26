#pragma once
#include <iostream>
#include <fstream>
#include <chrono>
#include <random>

enum {
    CLS = 0x00E0,
    RET = 0x00EE,
    JMP = 0x1,
    CALL = 0x2,
    SEimm = 0x3,
    SNEimm = 0x4,
    SE = 0x5,
    LD = 0x6,
    ADD = 0x7
};

struct Chip8 {
    uint8_t memory[4096];
    uint8_t videoRam[64*32];

    uint8_t registers[16];
    uint8_t soundTimer, delayTimer, SP;
    uint16_t PC, I;
    uint16_t stack[16];
    
    std::mt19937 rnd;

    Chip8() :rnd(std::chrono::steady_clock::now().time_since_epoch().count()) {}

    uint16_t startAddress = 0x200;

    void loadROM(const char* filename);
    void processInput();
    void execIns();

};