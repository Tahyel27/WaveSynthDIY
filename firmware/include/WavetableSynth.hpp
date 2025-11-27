#pragma once 

#include <cmath>
#include <array>
#include "AudioInterface.hpp"

//header file that contains static const arrays of wavetables
#include "wave_tables.hpp"

struct sineSynthT
{
    uint64_t samplecounter;

    int16_t operator()(double f, int amp, uint64_t SPS)
    {
        double t = double(samplecounter) / double(SPS);
        samplecounter += 1 * f;
        if (samplecounter > SPS)
        {
            samplecounter = samplecounter - SPS;
        }

        return amp * sin(2* t * M_PI);
    }
};

class WavetableSynth : public AudioSource
{
private:
    using float_t = float;

    float_t s_counter;
    std::array<float_t, 4> s_counters;
    int voices;

    float_t detune;

    float_t morph_counter;

    float_t morphdir;

    float_t freq;

    float_t s_inc;

    sineSynthT sine;
    
    int16_t sampleLinear(float_t x);

    int16_t sampleLinearSecond(float_t x);

    int16_t *wt;

public:

    virtual void audioCallback(AudioBuffer buffer) override;

    WavetableSynth(/* args */);
    ~WavetableSynth();

    void setFreq(float f);
};
