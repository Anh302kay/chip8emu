#include <iostream>
#include <fstream>
#include <chrono>
#include <cstring>
#include <cstdint>

#include "chip8.hpp"

#ifdef __PC
#include "platformSDL.hpp"
#elif __3DS__
#include "platformCTR.hpp"
#endif


int main(int argc, char* argv[])
{
    platformClass platform;

    Chip8 chip8;
    chip8.loadROM("/Breakout (Brix hack) [David Winter, 1997].ch8");

    bool gameRunning = true;
    auto previous = std::chrono::high_resolution_clock::now();
    auto current = previous;

    int timeStep = 2500;
    std::chrono::duration<double, std::milli> accumulator;

    while(gameRunning) {
        previous = current;
        current = std::chrono::high_resolution_clock::now();
        accumulator += current - previous;

        while(accumulator > std::chrono::microseconds(timeStep)) {
            if(!gameRunning)
                break;

            platform.processInput(chip8, gameRunning);

            if(chip8.soundTimer > 0)
                platform.playSound();
            else 
                platform.stopSound();

            chip8.execIns();
            accumulator -= std::chrono::microseconds(timeStep);
        }
        platform.startFrame();
        platform.render(chip8.videoRam);

        platform.drawUI(chip8, timeStep);

        platform.endFrame();
    }

    return 0;
}