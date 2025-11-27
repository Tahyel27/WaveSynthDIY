#pragma once 

#include <cmath>
#include "AudioInterface.hpp"

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
    sineSynthT sine;
public:

    void audioCallback(AudioBuffer buffer) override;

    WavetableSynth(/* args */);
    ~WavetableSynth();
};
