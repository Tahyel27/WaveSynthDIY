#pragma once

#include <hardware/pio.h>
#include <hardware/gpio.h>
#include <array>
#include <algorithm>
#include <queue>
#include <optional>
#include <pico/stdlib.h>
#include <stdio.h>

#include "button_array.pio.h"

struct ButtonEvent
{
    int button;
    int deviceId;
    enum class Type{
        PRESSED,
        RELEASED
    };
    Type type;
};

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
    std::array<int, MAX_BUTTONS> buttons_prev;

    std::queue<int> pressedQueue;
    std::queue<int> releasedQueue;

    int deviceID;

    void button_array_program_init();
public:
    void poll();

    bool isPressed(int button);

    std::optional<ButtonEvent> getEvent();

    ButtonArray(uint datapin, uint clockpin, uint latchpin);
    ~ButtonArray();
};

