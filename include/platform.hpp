#pragma once
#include <iostream>
#include <cstdint>
class platform
{
public:
    virtual void processInput(bool* keypad);
    virtual void playSound();
    virtual void stopSound();
    virtual void render(uint8_t* videoRam);
};