#pragma once
#include <iostream>
#include <fstream>
#include <chrono>
#include <array>
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
    uint8_t memory[4096] = {0};
    uint8_t videoRam[64*32] = {0};

    uint8_t registers[16] = {0};
    uint8_t soundTimer = 0, delayTimer = 0, SP = 0;
    uint16_t PC = 0, I = 0;
    uint16_t stack[16] = {0};
    bool keypad[16] = {false};
    
    std::mt19937 rnd;
    uint8_t palette = 255;

    Chip8();

    void reset();
    void loadROM(const char* filename);
    void processInput();
    void execIns();

};