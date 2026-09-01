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
public:

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

    template<size_t N>
    void poll(staticQueue<Event, N> &event_queue)
    {
        pio.clear_fifos();

        uint32_t word = pio.get_blocking();

        int buttons_index = 0;

        const uint32_t mask = 0x7FFFFFFF; // 01111111

        for (size_t i = 0; i < 32; i++)
        {
            bool down = ((word << i | mask) == 0xFFFFFFFF); // checks if button i is currently pressed
            if (down)                                       // button is pressed now
            {
                // the previous state of this button
                bool down_prev = ((prev_state << i | mask) == 0xFFFFFFFF);
                // if it wasnt pressed register a new press
                if (!down_prev)
                {
                    event_queue.push(Event::button_press(i));
                }
            }
            else // button isnt pressed now
            {
                // the previous state of this button
                bool down_prev = ((prev_state << i | mask) == 0xFFFFFFFF);
                // if it was pressed before register a release
                if (down_prev)
                {
                    event_queue.push(Event::button_release(i));
                }
            }
        }

        prev_state = word;
    }
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
