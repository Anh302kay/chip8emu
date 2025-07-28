#include <iostream>
#include <cstdint>

#include "platform.hpp"

void platform::processInput(bool* keypad)
{
    std::cout << "processInput not implemented\n";
}
void platform::playSound()
{
    std::cout << "playSound not implemented\n";
}
void platform::stopSound()
{
    std::cout << "stopSound not implemented\n";
}
void platform::startFrame()
{
    std::cout << "startFrame not implenteded\n";
}
void platform::render(uint8_t* videoRam)
{
    std::cout << "render not implemented\n";
}
void platform::endFrame()
{
    std::cout << "endFrame not implenteded\n";
}