#pragma once
#include <3ds.h>
#include <citro3d.h>
#include <citro2d.h>
#include "platform.hpp"

#include "chip8.hpp"

#define platformClass platformCTR

enum 
{
    BUTTON_RESET = 0,
    BUTTON_PAUSE,
    BUTTON_UNPAUSE,
    BUTTON_YES,
    BUTTON_NO,
    BUTTON_SURE,
    BUTTON_LOADROM,
    NUMBUTTONS = 7
};

enum
{
    MENU_KEYPAD,
    MENU_COLOURS,
    MENU_SETTINGS,
    MENU_FILE
};

struct slider
{
    int width, height;
    int min, max;
    int x, y;
    int value;
    void render(u32 lineColour = C2D_Color32(1.f,1.f,1.f,1.f), u32 sliderColour = C2D_Color32(1.f,1.f,1.f,1.f));
    enum
    {
        RED,
        GREEN,
        BLUE
    };

};
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
    void parseString(C2D_Text& text, const char* str);
    C3D_RenderTarget* top;
    C3D_RenderTarget* bottom;
    C2D_Image screen;

    C2D_Text keypadText[16];
    C2D_Text buttons[NUMBUTTONS];
    C2D_Text pathText;
    C2D_TextBuf textUIBuf;
    C2D_TextBuf textBuf;
    C2D_Font font;
    ndspWaveBuf waveBuffer;
    touchPosition touch;
    slider RGBslider[3];
    

    float scale = 5.f;
    u16 colour = 0xFFFF;
    u16 pathX = 0;
    u16 pathW = 0;
    u8 settings = MENU_KEYPAD;
};