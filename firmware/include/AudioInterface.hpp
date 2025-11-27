#pragma once

#include <cstdint>

struct AudioBuffer
{
    uint32_t *buffer;
    int buffsize;
    uint64_t SPS;
    int maxamp;

    enum class Mode
    {
        MONO,
        LEFT,
        RIGHT
    };

    void write16bit(int i, uint16_t value, Mode mode)
    {
        if (mode == Mode::LEFT)
        {
            buffer[2 * i] = value << 16;
        }
        else if (mode == Mode::RIGHT)
        {
            buffer[2 * i + 1] = value << 16;
        }
        else
        {
            buffer[2 * i] = value << 16;
            buffer[2 * i + 1] = value << 16;
        }
    }
};

struct AudioSource
{
    virtual void audioCallback(AudioBuffer buffer) = 0;
};