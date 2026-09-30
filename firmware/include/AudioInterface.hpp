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

    void write16bit(int i, int16_t value, Mode mode)
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

    inline void write32bit_float(int i, float value, Mode mode)
    {
        constexpr int MAX = 2147483647;

        int val = static_cast<int32_t>(value * MAX * 0.9999);
        
        if (mode == Mode::LEFT)
        {
            buffer[2 * i] = val;
        }
        else if (mode == Mode::RIGHT)
        {
            buffer[2 * i + 1] = val;
        }
        else
        {
            buffer[2 * i] = val;
            buffer[2 * i + 1] = val;
        }
    }
};

struct AudioSource
{
    virtual void audioCallback(AudioBuffer buffer) = 0;
};