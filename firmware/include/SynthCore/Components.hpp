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
        AMPLIFIER,
        ADSR,
        SVFLP,
        DELAY
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

    struct ADSRData
    {
        float_t timer = 0;

        enum class State
        {
            IDLE,
            ATTACK,
            HOLD,
            DECAY,
            SUSTAIN,
            RELEASE
        };
        State state = State::ATTACK;

        float_t attack = 0.05;
        float_t hold = 0.0;
        float_t decay = 0.1;
        float_t sustain = 0.8;
        float_t release = 0.0;
    };
    
    struct SVFData
    {
        float_t z1 = 0;
        float_t z2 = 0;

        float_t fcut = 2500;
        float_t fenv = 0;
        float_t Q = 0.1;
        ModInput modulation = ModInput{-1, 0};
        ModInput input;
    };

    struct DelayData
    {
        static constexpr unsigned int LENGTH = 15000;
        int delay = 14000;
        int write_index = 0;

        std::array<float_t, LENGTH> delayLine;
        ModInput input;

        float_t gain = 0.2;
    };

    struct Node
    {
        NodeType type;
        int dataIndex;
        int outputBuffer;
    };

    struct NodeOrder
    {
        std::array<Node, MAX_GRAPH_NODES> data;
        int nodeCount;
    };
    // defines an array of component data for each voice, sicne in different voices oscillators could be in a different phase for example
    struct Data
    {
        static constexpr int WTOscNum = 2;
        std::array<WTOscData, WTOscNum> WTOscArr;

        static constexpr int OSCNum = 4;
        std::array<SineOscData, OSCNum> SineOscArr;
        std::array<SawOscData, OSCNum> SawOscArr;

        static constexpr int AmplifierNum = 5;
        std::array<AmplifierData, AmplifierNum> AmplifierArr;

        static constexpr int ADSRNum = 5;
        std::array<ADSRData, ADSRNum> ADSRArr;

        static constexpr int SVFNum = 4;
        std::array<SVFData, SVFNum> SVFArr;
    };

    void processNode(NodeType type, int nodeID, Data &data, float_t *outbuffer, BufferPool *buffers);

    void processWTOsc(WTOscData * data, BufferPool * pool, float_t * outbuffer);
    void processSineOsc(SineOscData * data, BufferPool *pool, float_t *outbuffer);
    void processSawOsc(SawOscData * data, BufferPool *pool, float_t *outbuffer);
    void processAmplifier(const AmplifierData &data, BufferPool *pool, float_t *outbuffer);
    void processADSR(ADSRData *data, float_t *outbuffer);
    void processSVFLPData(SVFData *data, BufferPool *pool, float_t *outbuffer);
    void processDelayData(DelayData *data, BufferPool *pool, float_t *outbuffer);

    void globalDelay(DelayData *data, float_t *input, float_t *output, size_t bufferLength);
}