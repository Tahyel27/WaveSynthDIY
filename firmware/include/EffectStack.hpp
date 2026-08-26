#pragma once

#include "SynthCore/Common.hpp"
#include <cmath>

struct EffectStackConfig
{
    bool hard_clip = false;
    float_t hard_clip_gain = 2.0f;
};

struct EffectStack
{
    EffectStack(const EffectStackConfig &cfg) : m_config(cfg) {};

    //modifies the audio in place by applying the effects in the stack
    void apply_effects(float_t * audio_buffer)
    {
        //here apply the individual effects in the stack according to the configuration
        //modifies the audio in place

        //hard clipping
        if (m_config.hard_clip)
        {
            for (size_t i = 0; i < BUFFER_SIZE; i++)
            {
                audio_buffer[i] = std::clamp(audio_buffer[i] * m_config.hard_clip_gain, -1.0f, 1.0f);
            }
        }
        
    }

    void set_config(const EffectStackConfig &cfg) { m_config = cfg; };
private:
    EffectStackConfig m_config;
};
