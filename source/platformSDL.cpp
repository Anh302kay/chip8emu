#include <iostream>
#include <SDL3/SDL.h>
#include "platformSDL.hpp"

static void audioCallback(void *userdata, SDL_AudioStream *stream, int additional_amount, int total_amount)
{
    int currentSample;
    while(additional_amount > 0) {
        int16_t samples[128];
        const int total = SDL_min(additional_amount, sizeof(samples));

        for(int i = 0; i < total; i++) {
            samples[i] = ( SDL_sinf(currentSample*SDL_PI_F/2) > 0) ? 5000 : -5000 ;
            currentSample++;
        }

        additional_amount -= total;
        SDL_PutAudioStreamData(stream, samples, total);

    }
}

platformSDL::platformSDL()
    : window(NULL),
      renderer(NULL),
      screen(NULL),
      stream(NULL)
{
    SDL_SetAppMetadata("Chip8 Emulator", "0.0.1", "com.Anh302kay.CHIP8");

    if(!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        std::cout << "error could not init sdl3:" << SDL_GetError();
        return;
    }

    if(!SDL_CreateWindowAndRenderer("Chip8 Emulator", 640*2, 320*2, 0, &window, &renderer)) {
        std::cout << "error could not create window:" << SDL_GetError();
        return;
    }

    screen = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB332, SDL_TEXTUREACCESS_STREAMING, 64, 32);
    if(screen == NULL) {
        std::cout << "could not create tex: " << SDL_GetError();
    } 
    SDL_SetTextureScaleMode(screen, SDL_SCALEMODE_NEAREST);

    spec.channels = 1;
    spec.format = SDL_AUDIO_S16LE;
    spec.freq = 2000;

    stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, &audioCallback, NULL);

}

platformSDL::~platformSDL()
{
    SDL_DestroyTexture(screen);
    SDL_Quit();
}


void platformSDL::processInput(bool* keypad)
{
    const bool* keystate = SDL_GetKeyboardState(NULL);
    keypad[0] = keystate[SDL_SCANCODE_X];
    keypad[1] = keystate[SDL_SCANCODE_1];
    keypad[2] = keystate[SDL_SCANCODE_2];
    keypad[3] = keystate[SDL_SCANCODE_3];
    keypad[4] = keystate[SDL_SCANCODE_Q];
    keypad[5] = keystate[SDL_SCANCODE_W];
    keypad[6] = keystate[SDL_SCANCODE_E];
    keypad[7] = keystate[SDL_SCANCODE_A];
    keypad[8] = keystate[SDL_SCANCODE_S];
    keypad[9] = keystate[SDL_SCANCODE_D];
    keypad[10] = keystate[SDL_SCANCODE_Z];
    keypad[11] = keystate[SDL_SCANCODE_C];
    keypad[12] = keystate[SDL_SCANCODE_4];
    keypad[13] = keystate[SDL_SCANCODE_R];
    keypad[14] = keystate[SDL_SCANCODE_F];
    keypad[15] = keystate[SDL_SCANCODE_V];
}

void platformSDL::processInput(Chip8& chip8, bool& gameRunning)
{

    SDL_Event e;
    while(SDL_PollEvent(&e)) {
        if(e.type == SDL_EVENT_QUIT)
            gameRunning = false;
        if(e.type == SDL_EVENT_DROP_FILE) {
            chip8.reset();
            chip8.loadROM(e.drop.data);
        }
    }

    const bool* keystate = SDL_GetKeyboardState(NULL);
    chip8.keypad[0] = keystate[SDL_SCANCODE_X];
    chip8.keypad[1] = keystate[SDL_SCANCODE_1];
    chip8.keypad[2] = keystate[SDL_SCANCODE_2];
    chip8.keypad[3] = keystate[SDL_SCANCODE_3];
    chip8.keypad[4] = keystate[SDL_SCANCODE_Q];
    chip8.keypad[5] = keystate[SDL_SCANCODE_W];
    chip8.keypad[6] = keystate[SDL_SCANCODE_E];
    chip8.keypad[7] = keystate[SDL_SCANCODE_A];
    chip8.keypad[8] = keystate[SDL_SCANCODE_S];
    chip8.keypad[9] = keystate[SDL_SCANCODE_D];
    chip8.keypad[10] = keystate[SDL_SCANCODE_Z];
    chip8.keypad[11] = keystate[SDL_SCANCODE_C];
    chip8.keypad[12] = keystate[SDL_SCANCODE_4];
    chip8.keypad[13] = keystate[SDL_SCANCODE_R];
    chip8.keypad[14] = keystate[SDL_SCANCODE_F];
    chip8.keypad[15] = keystate[SDL_SCANCODE_V];
}

void platformSDL::playSound()
{
    SDL_ResumeAudioStreamDevice(stream);
}

void platformSDL::stopSound()
{
    SDL_PauseAudioStreamDevice(stream);
}

void platformSDL::startFrame()
{
    SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
    SDL_RenderClear(renderer);
}

void platformSDL::render(uint8_t* videoRam)
{
    SDL_Surface* surface;
    if(SDL_LockTextureToSurface(screen, NULL, &surface)) {
        memcpy(surface->pixels, videoRam, 2048);
        SDL_UnlockTexture(screen);
        surface = nullptr;
    }


    SDL_RenderTexture(renderer, screen, NULL, NULL);
}

void platformSDL::endFrame()
{
    SDL_RenderPresent(renderer);
}
