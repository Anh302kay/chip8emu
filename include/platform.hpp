#pragma once
#include <iostream>
#include <cstdint>

class platform
{
public:
    virtual void init();
    virtual void processInput(uint8_t* numpad);
    virtual void render(uint8_t* videoRam);
    virtual void quit();
private:

};