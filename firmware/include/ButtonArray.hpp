#pragma once

#include <hardware/pio.h>
#include <hardware/gpio.h>
#include <array>
#include <pico/stdlib.h>

#include "button_array.pio.h"

class ButtonArray
{
private:
    struct Pins
    {
        uint clock;
        uint latch;
        uint datain;
    };
    Pins pins;

    struct PIOdata
    {
        PIO pio;
        uint sm;
        uint offset;
    };
    PIOdata pio;

    static const uint MAX_BUTTONS = 256;
    std::array<char, MAX_BUTTONS> buttons_prev;

    uint deviceID;

    void button_array_program_init();
public:
    void poll();

    ButtonArray(uint datapin, uint clockpin, uint latchpin);
    ~ButtonArray();
};

