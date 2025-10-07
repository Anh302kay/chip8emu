#ifdef __3DS__
#include <3ds.h>
#include <citro3d.h>
#include <citro2d.h>
#include <filesystem>
#include <ranges>
#include <deque>
#include "platform.hpp"

#include "chip8.hpp"
#include "platformCTR.hpp"
#include "filesystem.hpp"

constexpr u8 textLookup[16] = { 1, 2, 3, 0xC, 4, 5, 6, 0xD, 7, 8, 9, 0xE, 0xA, 0, 0xB, 0xF};

static constexpr size_t posToTex(u16 index) 
{
    constexpr int width = 64;
    const int x = index % 64;
    const int y = index / 64;

    return ((((y >> 3) * (width >> 3) + (x >> 3)) << 6) + ((x & 1) | ((y & 1) << 1) | ((x & 2) << 1) | ((y & 2) << 2) | ((x & 4) << 2) | ((y & 4) << 3)));
}

static constexpr void fillBuffer(s8* buffer, int size) {
    for(int i = 0; i < size; i++) {
        buffer[i] = sinf((float)i*3.14/8.f) > 0 ? -50 : 50;
    }
}

template <typename T1, typename T2>
static constexpr bool touchedBox(const touchPosition& touch, const T1 x, const T1 y, const T2 w, const T2 h) {
    return touch.px > x && touch.px < x + w && touch.py > y && touch.py < y + h;
}  

void slider::render(u32 lineColour, u32 sliderColour)
{
    const int range = max-min;
    const float offset = (float)height/(float)range;

    C2D_DrawLine(x+width/2, y, lineColour, x+width/2, y+height, lineColour, 2, 0);
    C2D_DrawRectSolid(x, y + height - offset*value - 5.f, 0, width , 5, sliderColour);

}

void platformCTR::parseString(C2D_Text& text, const char* str)
{
    if(C2D_TextFontParse(&text, font, textBuf, str) != 0) {
        C2D_TextBufClear(textBuf);
        C2D_TextFontParse(&text, font, textBuf, str);
    }
    C2D_TextOptimize(&text);
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
    C3D_TexInit(screen.tex, 64, 32, GPU_RGBA5551);
    C3D_TexSetFilter(screen.tex, GPU_NEAREST, GPU_NEAREST);

    Tex3DS_SubTexture* subtex = new Tex3DS_SubTexture;
    subtex->width = 64;
    subtex->height = 32;
    subtex->top = 1.f;
    subtex->right = 1.f;
    screen.subtex = subtex;

    font = C2D_FontLoad("romfs:gfx/GravityBold8.bcfnt");
    textUIBuf = C2D_TextBufNew(128);
    for(int i = 0; i < 16; i++) {
        std::string num = std::format("{:x}", i);
        C2D_TextFontParse(&keypadText[i], font, textUIBuf, num.c_str());
        C2D_TextOptimize(&keypadText[i]);
    }

    textBuf = C2D_TextBufNew(1024);

    for (int i = 0; i < 3; i++) {
        RGBslider[i] = {
            .width = 10,
            .height = 150,
            .min = 0,
            .max = 31,
            .x = 20 + 50*i,
            .y = 10,
            .value = 31 
        };
    }

    C2D_TextFontParse(&buttons[BUTTON_RESET], font, textUIBuf, "RESET");
    C2D_TextOptimize(&buttons[BUTTON_RESET]);

    C2D_TextFontParse(&buttons[BUTTON_PAUSE], font, textUIBuf, "PAUSE");
    C2D_TextOptimize(&buttons[BUTTON_PAUSE]);

    C2D_TextFontParse(&buttons[BUTTON_UNPAUSE], font, textUIBuf, "UNPAUSE");
    C2D_TextOptimize(&buttons[BUTTON_UNPAUSE]);

    C2D_TextFontParse(&buttons[BUTTON_YES], font, textUIBuf, "YES");
    C2D_TextOptimize(&buttons[BUTTON_YES]);

    C2D_TextFontParse(&buttons[BUTTON_NO], font, textUIBuf, "NO");
    C2D_TextOptimize(&buttons[BUTTON_NO]);

    C2D_TextFontParse(&buttons[BUTTON_SURE], font, textUIBuf, "ARE YOU SURE?");
    C2D_TextOptimize(&buttons[BUTTON_SURE]);

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

void platformCTR::loadRom(Chip8& chip8, bool& gameRunning)
{

    FS_Archive sdmcArchive;
    FSUSER_OpenArchive(&sdmcArchive, ARCHIVE_SDMC, fsMakePath(PATH_EMPTY, ""));

    std::filesystem::path path = "/";
    
    std::vector<std::string> files = loadDirectory(path.string(), sdmcArchive);

    C2D_TextBuf fileTextBuf = C2D_TextBufNew(4096);
    C2D_Font liberationSans = C2D_FontLoad("romfs:/gfx/LiberationSans-Bold.bcfnt");
    std::deque<C2D_Text> fileText = loadDirList(files, liberationSans, fileTextBuf);
    u16 selectedFile = 0;
    u16 topEntry = 0; // top entry shown on screen
    u64 buttonDelay = 0;
    bool confirmBox = false;
    bool confirmOption = false;

    C2D_Text pathText;
    C2D_TextFontParse(&pathText, liberationSans, fileTextBuf, path.c_str());
    C2D_TextOptimize(&pathText);


    constexpr u32 white = C2D_Color32(255,255,255,255);
    constexpr u32 grey = C2D_Color32(150,150,150,255);
    constexpr u32 black = C2D_Color32(0,0,0,255);
    hidSetRepeatParameters(250, 30);
    while(gameRunning = aptMainLoop()) {
        hidScanInput();
        const u32 kDown = hidKeysDown();
        const u32 kHeld = hidKeysHeld();
        const u32 kRepeat = hidKeysDownRepeat();
        
        if(kRepeat & KEY_UP && selectedFile > 0) {
            selectedFile--;
            if(selectedFile < topEntry) {
                topEntry--;
                fileText.pop_back();
                C2D_Text text;
                C2D_TextFontParse(&text, liberationSans, fileTextBuf, files.at(selectedFile).c_str());
                C2D_TextOptimize(&text);
                fileText.push_front(text);
            }
        }

        if(kRepeat & KEY_DOWN) {
            selectedFile++;
            if(selectedFile >= files.size() && !files.empty())
                selectedFile = files.size()-1;

            if(selectedFile >= topEntry + 18) {
                topEntry++;
                fileText.pop_front();
                C2D_Text text;
                C2D_TextFontParse(&text, liberationSans, fileTextBuf, files.at(selectedFile).c_str());
                C2D_TextOptimize(&text);
                fileText.push_back(text);
            }
        }

        if(kDown & KEY_A && !files.empty()) {
            if(std::filesystem::is_directory(path.string() + files.at(selectedFile))) {
                path += files[selectedFile];
                files = loadDirectory(path.string(), sdmcArchive);
                fileText.clear();
                C2D_TextBufClear(fileTextBuf);
                fileText = loadDirList(files, liberationSans, fileTextBuf);
                selectedFile = 0;
                topEntry = 0;
                C2D_TextFontParse(&pathText, liberationSans, fileTextBuf, path.c_str());
                C2D_TextOptimize(&pathText);
            }
            else if(!confirmBox) {
                confirmBox = true;
                buttonDelay = osGetTime();
            }
        }

        if(kDown & KEY_B && path != "/") {
            path = getParentPath(path.string());
            files = loadDirectory(path.string(), sdmcArchive);
            fileText.clear();
            C2D_TextBufClear(fileTextBuf);
            fileText = loadDirList(files, liberationSans, fileTextBuf);
            selectedFile = 0;
            topEntry = 0;
            C2D_TextFontParse(&pathText, liberationSans, fileTextBuf, path.c_str());
            C2D_TextOptimize(&pathText);
        }

        if(confirmBox) {
            if(kDown & KEY_LEFT)
                confirmOption = true;

            if(kDown & KEY_RIGHT)
                confirmOption = false;

            if(kDown & KEY_A && buttonDelay + 100 < osGetTime()) {
                if(confirmOption) {
                    std::string totalPath = path.string() + files.at(selectedFile);
                    chip8.loadROM(totalPath.c_str());
                    break;
                }
                confirmBox = false;
            }
        }

        startFrame();
        C2D_TargetClear(bottom, C2D_Color32f(0.0f, 1.0f, 0.0f, 1.0f));
        C2D_SceneBegin(bottom);
        for(auto [index, text] : std::views::enumerate(fileText)) 
            C2D_DrawText(&text, C2D_WithColor, 20, 19 + 12 * index, 0.f, .6f, .6f, index == selectedFile - topEntry ? white : grey);
        
        C2D_DrawText(&pathText, C2D_WithColor, 15, 5, 0, 0.6f, 0.6f, black);

        if(confirmBox) {
            C2D_DrawRectSolid(33, 28, 0, 255, 90, black);
            C2D_DrawText(&buttons[BUTTON_SURE], C2D_WithColor | C2D_AlignCenter, 160, 40, 0, 0.8f, 0.8f, white);
            C2D_DrawText(&fileText[selectedFile - topEntry], C2D_WithColor | C2D_AlignCenter, 160, 60, 0, 0.6f, 0.6f, white);
            C2D_DrawRectSolid(85 + !confirmOption * 90, 77, 0, 50, 25, white);
            C2D_DrawRectSolid(87 + !confirmOption * 90, 79, 0, 46, 21, black);
            C2D_DrawText(&buttons[BUTTON_YES], C2D_WithColor | C2D_AlignCenter, 110, 80, 0, 0.7f, 0.7f, confirmOption ? white : grey);
            C2D_DrawText(&buttons[BUTTON_NO], C2D_WithColor | C2D_AlignCenter, 200, 80, 0, 0.7f, 0.7f, !confirmOption ? white : grey);
        }

        endFrame();

    }
    C2D_TextBufDelete(fileTextBuf);
    C2D_FontFree(liberationSans);
    FSUSER_CloseArchive(sdmcArchive);
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

    if(kDown & KEY_TOUCH) {
        float width = 0.f;
        float height = 0.f;
        C2D_TextGetDimensions(&buttons[chip8.paused ? BUTTON_UNPAUSE : BUTTON_PAUSE], 0.75f, 0.5f, &width, &height);
        if(touchedBox(touch, 30, 217, width, height))
            chip8.paused = !chip8.paused;

        C2D_TextGetDimensions(&buttons[BUTTON_RESET], 0.75f, 0.5f, &width, &height);
        if(touchedBox(touch, 200, 217, width, height))
            chip8.reset();

        for(int i = 0; i < 3; i++) {
            if(touchedBox(touch, 267, 18+65*i, 47, 47))
                settings = i;
        }

    }

    bool* keypad = chip8.keypad;    
    switch(settings) {
        case MENU_COLOURS:
            if(kHeld & KEY_TOUCH) {
                for(slider& slider : RGBslider)
                if(touchedBox(touch, slider.x-10, slider.y-10, slider.width+20, slider.height+10 )) {
                    const int pointY = touch.py - slider.y;
                    const float range = slider.max-slider.min;
                    const float offset = slider.height/range;
                    slider.value = slider.max - pointY/offset;
                    slider.value = slider.value > slider.max ? slider.max : slider.value < slider.min ? slider.max : slider.value; // limit to range
                    colour = RGBslider[slider::RED].value << 11 | RGBslider[slider::GREEN].value << 6 | RGBslider[slider::BLUE].value << 1 | 0x1;
                }
            }

            break;
        case MENU_KEYPAD:
            memset(keypad, 0, 16);
            keypad[2] = kHeld & KEY_UP;
            keypad[8] = kHeld & KEY_DOWN;
            keypad[4] = kHeld & KEY_LEFT;
            keypad[6] = kHeld & KEY_RIGHT;

            if(!kHeld)
                return;
                
            for(int i = 0; i < 16; i++) {
                const int x = (i%4);
                const int y = (i/4);
                const int xPos = 20+x*60;
                const int yPos = y * 40 + y * 11 + 11;
                if(touchedBox(touch, xPos, yPos, 40, 40))
                    keypad[textLookup[i]] = true;
            }
            break;
        default:
            break;
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
    u16* texData = (u16*)screen.tex->data; 
    for(int i = 0; i < 2048; i++) {
        texData[posToTex(i)] = videoRam[i] == 255 ? colour : 0x1;
    }
    C2D_DrawImageAt(screen, 0, 0, 0, nullptr, scale, scale);
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

    switch(settings) {
        case MENU_FILE:
            
            break;
        case MENU_COLOURS:
            for(int i = 0; i < 3; i++) {
                RGBslider[i].render(white, white);
                C2D_Text value;
                parseString(value, std::to_string(RGBslider[i].value).c_str());
                C2D_DrawText(&value, C2D_WithColor | C2D_AlignCenter, RGBslider[i].x+10, RGBslider[i].y + RGBslider[i].height + 20, 0, .75f, .75f, white);
            }
            break;
        case MENU_KEYPAD:
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
            break;
        default:
            break;
    }

    for(int i = 0; i < 3; i++) {
        C2D_DrawRectSolid(267,18+65*i , 0, 47, 47, white);
    }
    C2D_DrawLine(317, 18+65*settings-5, white, 317, 18+65*settings+52, white, 2, 0);

    C2D_DrawLine(0, 215, white, 320, 215, white, 1, 0);
    C2D_DrawText(&buttons[chip8.paused ? BUTTON_UNPAUSE : BUTTON_PAUSE], C2D_WithColor, 30, 217, 0, .75f, .75f, grey);
    C2D_DrawText(&buttons[BUTTON_RESET], C2D_WithColor, 200, 217, 0, .75f, .75f, grey);

    C2D_DrawLine(260, 0, white, 260, 215, white, 2, 0);
}
#endif