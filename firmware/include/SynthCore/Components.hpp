#pragma once

#include <array>
#include <algorithm>
#include "SynthCore/Common.hpp"

namespace Synth
{

    struct ModInput
    {
        int bufID = -1;
        float_t v = 0;
    };

    enum class NodeType {
        WTOSCILLATOR,
        SINEOSCILLATOR,
        SAWOSCILLATOR,
        AMPLIFIER
    };

    struct DataHolder
    {
        NodeType type;
        std::array<ModInput, 10> data;
    };

    struct WTOscData
    {
        float_t phaseCounterA = 0;
        float_t phaseCounterB = 0;
        float_t phaseCounterC = 0;

        int wtIndex = 0;
        int unison = 1;
        bool useFm = false;

        ModInput detune;
        ModInput phaseDistort;
        ModInput phaseDistMod;
        ModInput freq = ModInput{-1, 440};
        ModInput morph;

    };

    struct SineOscData
    {
        float_t phaseCounter = 0;

        ModInput freq;
        ModInput phaseDistort;
        ModInput phaseDistMod;
    };

    struct SawOscData
    {
        float_t phaseCounter = 0;

        ModInput freq;
        ModInput phaseDistort;
        ModInput phaseDistMod;
    };

    struct AmplifierData
    {
        ModInput input;
        ModInput amount;
    };
    
    void processWTOsc(WTOscData * data, BufferPool * pool, float_t * outbuffer);
    void processSineOsc(SineOscData * data, BufferPool *pool, float_t *outbuffer);
    void processSawOsc(SawOscData * data, BufferPool *pool, float_t *outbuffer);
    void processAmplifier(const AmplifierData &data, BufferPool *pool, float_t *outbuffer);
}