#pragma once

#include <array>
#include <bitset>
#include <algorithm>
#include <tuple>
#include "AudioInterface.hpp"
#include "Components.hpp"
#include "SynthCore/Common.hpp"

namespace Synth
{
    
struct VoiceBuffers
{
    std::array<float_t, BUFFER_SIZE*VOICE_COUNT> data;

    inline float_t * get(int voice)
    {
        return &data[BUFFER_SIZE * voice];
    }
};



class SynthEngine : public AudioSource
{
private:
    BufferPool bufferPool;    

    std::array<NodeOrder, VOICE_COUNT> nodeOrderArray;

    int nodeCount = 0;

    std::array<Data, VOICE_COUNT> voiceData;

    VoiceBuffers voiceOutputs;

    std::bitset<VOICE_COUNT> activeVoices;

    DelayData delayLine;
    bool useDelay = false;

    void outputFromVoices(AudioBuffer buffer);

    void processGraph();

    void processChunk(int chunk, int voice);

 public:
    SynthEngine(/* args */){};
    ~SynthEngine(){};

    void loadData(const Data &data_);

    void loadVoiceData(const Data &data_, const NodeOrder &order_, int voice);

    std::tuple<Data&, NodeOrder&> getDataForVoiceRef(int voice);

    void startVoice(int voice);

    void stopVoice(int voice);

    bool isVoiceActive(int voice);

    void setDelay(bool state);

    void loadOrdering(const std::array<Node, MAX_GRAPH_NODES> &ordering, int nodes);

    virtual void audioCallback(AudioBuffer Buffer) override;
};

}