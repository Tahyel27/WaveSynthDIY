#pragma once

#include <AudioInterface.hpp>
#include <tuple>
#include <cstdint>
#include <stdio.h>

struct ADSR : public AudioSource
{
    AudioSource *source;

    enum class State {
        WAITING,
        ATTACK,
        HOLD,
        DECAY,
        SUSTAIN,
        RELEASE
    };

    State state = State::WAITING;

    uint32_t counter;

    float attack = 0.005;
    float hold = 0;
    float decay = 0.6;
    float sustain = 0;
    float release = 0;
    
    float value = 0;

    uint64_t SPS;
    
    int buffsize;

    void setSource(AudioSource *src) {source = src;};

    std::tuple<float, float> getBounds()
    {
        float a = 0;
        float b = 0;
        switch (state)
        {
        case State::ATTACK:
            a = value;
            b = a + static_cast<float>(buffsize)/(attack*static_cast<float>(SPS));
            break;
        case State::DECAY:
            a = value;
            b = a - static_cast<float>(buffsize)/(decay*static_cast<float>(SPS));
        default:
            break;
        }
        
        return {a, b};
    }

    void handleStateChange()
    {
        if (state == State::ATTACK)
        {
            if (counter > attack * static_cast<float>(SPS))
            {
                state = State::DECAY;
                value = 1;
                counter = 0;
            }
        }
        else if (state == State::DECAY)
        {
            if (value < 0)
            {
                state = State::WAITING;
            }
            
        }
        
        
    }

    virtual void audioCallback(AudioBuffer buffer) override 
    {
        source->audioCallback(buffer);
        
        SPS = buffer.SPS;
        buffsize = buffer.buffsize;
        auto mult = 1/static_cast<float>(1024);
        auto [a, b] = getBounds();
        for (int i = 0; i < buffsize; i++)
        {
            int32_t tmp = buffer.buffer[i*2];
            float val = static_cast<float>(static_cast<int16_t> (tmp >> 16));
            float lerp = a + i*(b - a)/1024;
            if (lerp > 1)
            {
                lerp = 1;
            }
            auto mod = 1*lerp*static_cast<float>(val);
            buffer.buffer[i*2] = static_cast<int16_t>(mod) << 16;
            buffer.buffer[i*2 + 1] = static_cast<int16_t>(mod) << 16;
        }
        value = b;
        counter += buffsize;
        handleStateChange();
    }

    void trigger(){
        state = State::ATTACK;
        counter = 0;
    };
};
