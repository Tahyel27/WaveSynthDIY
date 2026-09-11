#pragma once

//Handles its own pio because it needs to acces exact settings in the pio
//like the program needs to be loaded at address 0
#include "encoder.pio.h"
#include "pico/stdlib.h"
#include "hardware/pio.h"

#include "StaticQueue.hpp"
#include "Events.hpp"

#include <stdio.h>
#include <optional>

class Encoder
{
    int id = -1;
    //encoder states
    int new_value, old_value = 0;
    int last_delta = -1;
    //pins: A at pin_ab, B at pin_ab + 1
    uint pin_ab = 0;
    //PIO, program will be loaded at adress 0
    PIO m_pio = nullptr;
    uint sm;

    Encoder() = default;

    void release()
    {
        if (m_pio != nullptr)
        {
            pio_sm_set_enabled(m_pio, sm, false);
            pio_sm_unclaim(m_pio, sm);
            m_pio = nullptr;
        }
    }
public:
    Encoder(Encoder &&other) noexcept : 
        new_value(other.new_value), old_value(other.old_value), last_delta(other.last_delta),
        id(other.id),
        sm(other.sm), m_pio(other.m_pio)
    {
        other.id = -1;
        other.m_pio = nullptr;
    }

    Encoder(const Encoder&) = delete;
    Encoder& operator=(const Encoder&) = delete;

    //acquire the first possible encoder for a given PIO
    //this will have the sm = 0
    static std::optional<Encoder> acquire_first(PIO pio, uint pin_ab, int id) 
    {
        Encoder encoder;
        
        int offset = pio_add_program(pio, &encoder_program);
        if (offset != 0)
            return std::nullopt;

        encoder.m_pio = pio;
        encoder.sm = 0;
        encoder.pin_ab = pin_ab;
        encoder.id = id;

        encoder_program_init(encoder.m_pio, encoder.sm, encoder.pin_ab, 0);

        return encoder;
    }

    static std::optional<Encoder> acquire_other(const Encoder &first, uint pin_ab, uint sm, int id)
    {
        Encoder encoder;
        
        if (sm == 0)
            return std::nullopt;

        if (first.m_pio == nullptr)
            return std::nullopt;

        if (first.id == id)
            return std::nullopt;
        
        encoder.m_pio = first.m_pio;
        encoder.sm = sm;
        encoder.pin_ab = pin_ab;
        encoder.id = id;

        encoder_program_init(encoder.m_pio, encoder.sm, encoder.pin_ab, 0);

        return encoder;
    }

    template<size_t N>
    void poll(staticQueue<Event, N> &event_queue)
    {
        if (m_pio == nullptr)
            return;

        new_value = encoder_get_count(m_pio, sm);

        auto delta = new_value - old_value;

        if (new_value != old_value || delta != last_delta)
        {
            last_delta = delta;
            event_queue.push(Event::encoder_turn(id, delta));
        }

        old_value = new_value;
    }

    ~Encoder() 
    {
        release();
    }
};

