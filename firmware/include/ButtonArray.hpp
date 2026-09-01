#pragma once

#include <hardware/pio.h>
#include <hardware/gpio.h>
#include <array>
#include <algorithm>
#include <queue>
#include <optional>
#include <pico/stdlib.h>
#include <stdio.h>
#include <StaticQueue.hpp>
#include <Events.hpp>
#include <PioHandler.hpp>

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
    ButtonArray() = default;

    struct Pins
    {
        uint clock;
        uint latch;
        uint datain;
    };
    Pins pins;

    PioHandler pio;

    uint32_t prev_state = 0;

    std::queue<int> pressedQueue;
    std::queue<int> releasedQueue;

    int deviceID;
public:
    void poll();

    bool isPressed(int button);

    static std::optional<ButtonArray> claim(uint datapin, uint clockpin, uint latchpin);

    std::optional<ButtonEvent> getEvent();

    ButtonArray(ButtonArray &&other) : pio(std::move(other.pio)), prev_state(other.prev_state) {};
    ButtonArray &operator=(ButtonArray &&other) 
    {
        pio = std::move(other.pio);
        prev_state = other.prev_state;
        return *this;
    }
    ~ButtonArray();
};

struct RotaryEncoder
{
    uint pinA;
    uint pinB;
    bool state = false;
    bool A;
    bool B;
};

class EncoderArray
{
    struct Pins
    {
        uint clock;
        uint latch;
        uint datain;
    };
    Pins pins;

    struct PioData
    {
        PIO pio;
        uint sm;
        uint offset;
    };
    PioData pio;

    static constexpr int ENCODER_COUNT = 4;
    std::array<RotaryEncoder, ENCODER_COUNT> encoders;

    void init_prorgram();

    int read_encoder(int i, uint32_t word);

    bool pollEncoder(int i, Event &ev, uint32_t word);

    uint32_t poll();

    void populate_encoders();

public:
    EncoderArray(uint datapin, uint latchpin, uint clockpin);
    ~EncoderArray(){};
    
    template<size_t N>
    void pollEvents(staticQueue<Event, N> &evqueue)
    {
        uint32_t polled = poll();
        for (size_t i = 0; i < ENCODER_COUNT; i++)
        {
            Event tmp;
            if (pollEncoder(i, tmp, polled))
            {
                evqueue.push(tmp);
            }
        }
    };

};
