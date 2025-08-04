#pragma once
#include <3ds.h>
#include <citro3d.h>
#include <citro2d.h>
#include "platform.hpp"

#include "chip8.hpp"

#define platformClass platformCTR

class platformCTR : platform
{
public:
    platformCTR();
    ~platformCTR();

    void loadRom(Chip8& chip8, bool& gameRunning);

    void processInput(bool* keypad);
    void processInput(Chip8& chip8, bool& gameRunning);
    void playSound();
    void stopSound();
    void startFrame();
    void render(uint8_t* videoRam);
    void endFrame();
    void drawUI(Chip8& chip8, int& timeStep);
private:
    C3D_RenderTarget* top;
    C3D_RenderTarget* bottom;
    C2D_Image screen;

    C2D_Text keypadText[16];
    C2D_TextBuf textBuf;
    C2D_Font font;
    ndspWaveBuf waveBuffer;
    touchPosition touch;

    bool blackPalette = false;
    int scale;
};