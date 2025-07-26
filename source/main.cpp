#include <iostream>
#include <fstream>
#include <chrono>
#include <cstring>
#include <cstdint>
#include <SDL3/SDL.h>

#include "chip8.hpp"

constexpr auto timeStep = std::chrono::milliseconds(16);

int main(int* argc, char* argv[])
{
    SDL_SetAppMetadata("Chip8 Emulator", "0.0.1", "com.Anh302kay.CHIP8");

    if(!SDL_Init(SDL_INIT_VIDEO)) {
        std::cout << "error could not init sdl3:" << SDL_GetError();
    }

    SDL_Window* window;
    SDL_Renderer* renderer;
    
    if(!SDL_CreateWindowAndRenderer("Chip8 Emulator", 640, 320, 0, &window, &renderer)) {
        std::cout << "error could not create window:" << SDL_GetError();
    }

    Chip8 chip8;
    memset(chip8.videoRam, 0, sizeof(chip8.videoRam));
    chip8.loadROM("IBM Logo.ch8");
    chip8.videoRam[500] = 255;

    SDL_Texture* screen = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB332, SDL_TEXTUREACCESS_STREAMING, 64, 32);
    SDL_SetTextureScaleMode(screen, SDL_SCALEMODE_NEAREST);

    bool gameRunning = true;

    auto previous = std::chrono::high_resolution_clock::now();
    auto current = previous;
    // auto accumulator = previous;
    // double accumulator = 0;
    std::chrono::duration<double, std::milli> accumulator;

    while(gameRunning) {
        previous = current;
        current = std::chrono::high_resolution_clock::now();
        accumulator += current - previous;

        while(accumulator > timeStep) {
            SDL_Event e;
            while(SDL_PollEvent(&e)) {
                if(e.type == SDL_EVENT_QUIT)
                    gameRunning = false;
            }
            chip8.execIns();
            accumulator -= timeStep;
        }

        SDL_Surface* surface;
        if(SDL_LockTextureToSurface(screen, NULL, &surface)) {
            memcpy(surface->pixels, chip8.videoRam, sizeof(chip8.videoRam));
            SDL_UnlockTexture(screen);
        }

        SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
        SDL_RenderClear(renderer);
        SDL_RenderTexture(renderer, screen, NULL, NULL);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(screen);
    SDL_Quit();
    return 0;
}