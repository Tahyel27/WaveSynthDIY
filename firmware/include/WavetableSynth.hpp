#pragma once 

#include <cmath>
#include <array>
#include "AudioInterface.hpp"

//header file that contains static const arrays of wavetables
#include "wavetable_data.hpp"

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

    //fm params
    bool use_fm = false;
    float a = 1;
    int k1 = 1;

    inline float_t getSine(float_t x); //returns a sine in the range of -1 1

    inline float_t fmshiftCounter(int counter);

    inline int16_t sampleTableLinear(const int16_t *t, float_t x);

    int wt_index;
    int band_index;

    int getBand(float f);
public:

    virtual void audioCallback(AudioBuffer buffer) override;

    WavetableSynth(/* args */);
    ~WavetableSynth();

    void setFreq(float f);
};

inline float_t WavetableSynth::getSine(float_t x)
{
    return sinf(x*M_PI*2 / 1024);
}

inline float_t WavetableSynth::fmshiftCounter(int counter)
{
    float_t shifted = s_counters[counter] + a*1024*getSine(k1*s_counters[counter]);
    if (shifted > 1024)
    {
        shifted -= 1024;
    }
    else if(shifted < 0)
    {
        shifted += 1024;
    }

    return shifted;
}

inline int16_t WavetableSynth::sampleTableLinear(const int16_t *t, float_t x)
{
    int i = static_cast<int>(floor(x));
    float_t f = x - i;
    return (1 - f) * t[i] + f * t[i + 1];
}