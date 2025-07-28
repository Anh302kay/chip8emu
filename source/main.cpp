#include <iostream>
#include <fstream>
#include <chrono>
#include <cstring>
#include <cstdint>
#include <SDL3/SDL.h>

#include "chip8.hpp"

#ifdef __PC
#include "platformSDL.hpp"
#endif

constexpr auto timeStep = std::chrono::microseconds(2500);

int main(int argc, char* argv[])
{
    platformClass platform;

    Chip8 chip8;
    memset(chip8.videoRam, 0, sizeof(chip8.videoRam));
    chip8.loadROM("Breakout (Brix hack) [David Winter, 1997].ch8");
    // chip8.videoRam[500] = 255;

    bool gameRunning = true;
    auto previous = std::chrono::high_resolution_clock::now();
    auto current = previous;

    std::chrono::duration<double, std::milli> accumulator;

    while(gameRunning) {
        previous = current;
        current = std::chrono::high_resolution_clock::now();
        accumulator += current - previous;

        while(accumulator > timeStep) {

            platform.processInput(chip8, gameRunning);
            
            if(chip8.soundTimer > 0)
                platform.playSound();
            else 
                platform.stopSound();

            chip8.execIns();
            accumulator -= timeStep;
        }

        platform.render(chip8.videoRam);
    }

    return 0;
}