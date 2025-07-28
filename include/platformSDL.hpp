#pragma once
#include <SDL3/SDL.h>
#include "platform.hpp"

#include "chip8.hpp"

#define platformClass platformSDL

class platformSDL : platform
{
public:
    platformSDL();
    ~platformSDL();

    void processInput(bool* keypad);
    void processInput(Chip8& chip8, bool& gameRunning);
    void playSound();
    void stopSound();
    void render(uint8_t* videoRam);
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* screen;
    SDL_AudioSpec spec;
    SDL_AudioStream* stream;
};