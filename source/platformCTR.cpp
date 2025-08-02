#ifdef __3DS__
#include <3ds.h>
#include <citro3d.h>
#include <citro2d.h>
#include "platform.hpp"

#include "chip8.hpp"
#include "platformCTR.hpp"

static constexpr size_t posToTex(u16 index) 
{
    constexpr int width = 64;
    constexpr int height = 32;
    const int x = index % 64;
    const int y = index / 64;

    return ((((y >> 3) * (width >> 3) + (x >> 3)) << 6) + ((x & 1) | ((y & 1) << 1) | ((x & 2) << 1) | ((y & 2) << 2) | ((x & 4) << 2) | ((y & 4) << 3)));
}

static void fillBuffer(s8* buffer, int size) {
    for(int i = 0; i < size; i++) {
        buffer[i] = sinf((float)i*3.14/8.f) > 0 ? -50 : 50;
    }
    DSP_FlushDataCache(buffer, size);
}

platformCTR::platformCTR()
{
    gfxInitDefault();
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
    C2D_Prepare();
    
    top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);

    img.tex = new C3D_Tex;
    C3D_TexInitVRAM(img.tex, 64, 32, GPU_RGB565);
    C3D_TexSetFilter(img.tex, GPU_NEAREST, GPU_NEAREST);
    Tex3DS_SubTexture* subtex = new Tex3DS_SubTexture;
    subtex->width = 64;
    subtex->height = 32;
    subtex->top = 1.f;
    subtex->right = 1.f;
    img.subtex = subtex;

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
}

platformCTR::~platformCTR()
{
    C3D_TexDelete(img.tex);
    delete img.subtex;
    delete img.tex;
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
    // memcpy(img.tex, videoRam, 2048);
    u16* texData = (u16*)img.tex->data; 
    for(int i = 0; i < 2048; i++) {
        texData[posToTex(i)] = videoRam[i] == 255 ? 0xFFFF : 0;
    }
    C2D_DrawImageAt(img, 0, 0, 0, nullptr, 5.f, 5.f);
}
void platformCTR::endFrame()
{
    C3D_FrameEnd(0);
}

void platformCTR::drawUI(Chip8& chip8, int& timeStep)
{

}
#endif