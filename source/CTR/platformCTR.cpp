#ifdef __3DS__
#include <3ds.h>
#include <citro3d.h>
#include <citro2d.h>
#include "platform.hpp"

#include "chip8.hpp"
#include "platformCTR.hpp"

constexpr u8 textLookup[16] = { 1, 2, 3, 0xC, 4, 5, 6, 0xD, 7, 8, 9, 0xE, 0xA, 0, 0xB, 0xF};

static constexpr size_t posToTex(u16 index) 
{
    constexpr int width = 64;
    constexpr int height = 32;
    const int x = index % 64;
    const int y = index / 64;

    return ((((y >> 3) * (width >> 3) + (x >> 3)) << 6) + ((x & 1) | ((y & 1) << 1) | ((x & 2) << 1) | ((y & 2) << 2) | ((x & 4) << 2) | ((y & 4) << 3)));
}

static constexpr void fillBuffer(s8* buffer, int size) {
    for(int i = 0; i < size; i++) {
        buffer[i] = sinf((float)i*3.14/8.f) > 0 ? -50 : 50;
    }
}

platformCTR::platformCTR()
{
    gfxInitDefault();
    romfsInit();
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
    C2D_Prepare();
    
    top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
    bottom = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);

    screen.tex = new C3D_Tex;
    C3D_TexInit(screen.tex, 64, 32, GPU_RGB565);
    C3D_TexSetFilter(screen.tex, GPU_NEAREST, GPU_NEAREST);
    // GX_MemoryFill((u32*)screen.tex, 0, (u32*)(screen.tex+64*32*2), GX_FILL_16BIT_DEPTH | GX_FILL_TRIGGER, NULL, 0, NULL, 0);
    // gspWaitForPSC0();
    Tex3DS_SubTexture* subtex = new Tex3DS_SubTexture;
    subtex->width = 64;
    subtex->height = 32;
    subtex->top = 1.f;
    subtex->right = 1.f;
    screen.subtex = subtex;

    font = C2D_FontLoad("romfs:gfx/GravityBold8.bcfnt");
    textUIBuf = C2D_TextBufNew(4096);
    for(int i = 0; i < 16; i++) {
        std::string num = std::format("{:x}", i);
        C2D_TextFontParse(&keypadText[i], font, textUIBuf, num.c_str());
        C2D_TextOptimize(&keypadText[i]);
    }

    C2D_TextFontParse(&buttons[BUTTON_RESET], font, textUIBuf, "RESET");
    C2D_TextOptimize(&buttons[BUTTON_RESET]);

    C2D_TextFontParse(&buttons[BUTTON_PAUSE], font, textUIBuf, "PAUSE");
    C2D_TextOptimize(&buttons[BUTTON_PAUSE]);

    ndspInit();
    ndspSetOutputMode(NDSP_OUTPUT_MONO);
    ndspChnSetInterp(0, NDSP_INTERP_LINEAR);
    ndspChnSetRate(0, 8000);
    ndspChnSetFormat(0, NDSP_FORMAT_MONO_PCM8);

    float mix[12];
    memset(mix, 0, sizeof(mix));
    mix[0] = 1.f;
    ndspChnSetMix(0, mix);

    waveBuffer.nsamples = 8000;
    s8* audioBuffer = (s8*)linearAlloc(waveBuffer.nsamples);
    waveBuffer.data_pcm8 = audioBuffer;
    
    fillBuffer(audioBuffer, waveBuffer.nsamples);
    DSP_FlushDataCache(audioBuffer, waveBuffer.nsamples);


}

platformCTR::~platformCTR()
{
    C2D_FontFree(font);
    C2D_TextBufDelete(textUIBuf);
    C3D_TexDelete(screen.tex);
    delete screen.subtex;
    delete screen.tex;
    C2D_Fini();
    C3D_Fini();
    gfxExit();
}

void loadRom(Chip8& chip8, bool& gameRunning)
{

}

void platformCTR::processInput(bool* keypad)
{

}
void platformCTR::processInput(Chip8& chip8, bool& gameRunning)
{
    gameRunning = aptMainLoop();
    if(!gameRunning)
        return;
    hidScanInput();
    hidTouchRead(&touch);
    const u32 kDown = hidKeysDown();
    const u32 kHeld = hidKeysHeld();
    bool* keypad = chip8.keypad;

    memset(keypad, 0, 16);

    keypad[2] = kHeld & KEY_UP;
    keypad[8] = kHeld & KEY_DOWN;
    keypad[4] = kHeld & KEY_LEFT;
    keypad[6] = kHeld & KEY_RIGHT;

    if(!kHeld)
        return;
    for(int i = 0; i < 16; i++)
    {
        const int x = (i%4);
        const int y = (i/4);
        const int xPos = 20+x*60;
        const int yPos = y * 40 + y * 11 + 11;
        if(touch.px > xPos && touch.px < xPos+40 && touch.py > yPos && touch.py < yPos+40)
            keypad[textLookup[i]] = true;
    }
    
}
void platformCTR::playSound()
{
    if(waveBuffer.status != NDSP_WBUF_PLAYING)
        ndspChnWaveBufAdd(0, &waveBuffer);

    ndspChnSetPaused(0, false);
}
void platformCTR::stopSound()
{
    ndspChnSetPaused(0, true);
}
void platformCTR::startFrame()
{
    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
    C2D_TargetClear(top, C2D_Color32f(0.0f, 1.0f, 0.0f, 1.0f));
    C2D_SceneBegin(top);
}
void platformCTR::render(uint8_t* videoRam)
{
    // memcpy(screen.tex, videoRam, 2048);
    u16* texData = (u16*)screen.tex->data; 
    for(int i = 0; i < 2048; i++) {
        texData[posToTex(i)] = videoRam[i] == 255 ? 0xFFFF : 0;
    }
    C2D_DrawImageAt(screen, 0, 0, 0, nullptr, 5.f, 5.f);
}
void platformCTR::endFrame()
{
    C3D_FrameEnd(0);
}

void platformCTR::drawUI(Chip8& chip8, int& timeStep)
{
    C2D_TargetClear(bottom, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
    C2D_SceneBegin(bottom);
    
    constexpr u32 white = C2D_Color32(255,255,255,255);
    constexpr u32 grey = C2D_Color32(190,190,190,255);
    constexpr u32 black = C2D_Color32(0,0,0,255);
    for(int i = 0; i < 16; i++)
    {
        const int scale = chip8.keypad[textLookup[i]] * 5;
        constexpr int outlineSize = 2;
        const int x = (i%4);
        const int y = (i/4);
        const int xPos = 20+x*60- scale/2;
        const int yPos = y * 40 + y * 11 + 11 - scale/2;
        C2D_DrawRectSolid(xPos, yPos, 0, 40 + scale, 40 + scale, white);
        C2D_DrawRectSolid(xPos+outlineSize, yPos+outlineSize, 0, 40-outlineSize*2 +scale, 40-outlineSize*2 +scale, black);
        C2D_DrawText(&keypadText[textLookup[i]], C2D_AtBaseline | C2D_WithColor | C2D_AlignCenter, xPos+20 + scale/2, yPos+30 + scale , 0, 1.f * (chip8.keypad[textLookup[i]] ? 1.25f : 1.f) , 1.f * (chip8.keypad[textLookup[i]] ? 1.25f : 1.f) , white);
    }
    C2D_DrawText(&buttons[BUTTON_PAUSE], C2D_WithColor | C2D_AlignCenter, 60, 217, 0, .75f, .75f, grey);
    C2D_DrawLine(0, 215, white, 320, 215, white, 1, 0);

}
#endif