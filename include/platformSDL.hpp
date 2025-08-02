#pragma once
#include <SDL3/SDL.h>
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"
#include "platform.hpp"

#include "chip8.hpp"

#define platformClass platformSDL

class platformSDL : platform
{
public:
    platformSDL();
    ~platformSDL();

    void loadRom(Chip8& chip8, bool& gameRunning);
    void loadRomDialog(Chip8& chip8);

    void processInput(bool* keypad);
    void processInput(Chip8& chip8, bool& gameRunning);
    void playSound();
    void stopSound();
    void startFrame();
    void render(uint8_t* videoRam);
    void endFrame();
    void drawUI(Chip8& chip8, int& timeStep);
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* screen;
    SDL_AudioSpec spec;
    SDL_AudioStream* stream;
    bool blackPalette = false;
    bool settingsOpened = true;
    int scale = 20.f;
};