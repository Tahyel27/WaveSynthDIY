#pragma once

#include <array>
#include <bitset>
#include <algorithm>
#include <tuple>
#include <cmath>
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

    VoiceBuffers()
    {
        std::fill_n(data.begin(), data.size(), 0.00);
    }
};



class SynthEngine : public AudioSource
{
private:
    BufferPool bufferPool;    

    NodeOrder nodeOrder;

    int nodeCount = 0;

    Data data;

    std::array<float_t, BUFFER_SIZE> output_buffer;

    std::bitset<VOICE_COUNT> activeVoices;

    void outputFromVoices(AudioBuffer buffer);

    void processGraph();

    void processChunk(int chunk);

 public:
    SynthEngine(/* args */);
    ~SynthEngine(){};

    void loadData(const Data &data_);

    void loadVoiceData(const Data &data_, const NodeOrder &order_, int voice);

    std::tuple<Data&, NodeOrder&> getDataForVoiceRef(int voice);

    inline Data& getDataRef(int voice);

    void startVoice(int voice);

    void stopVoice(int voice);

    bool isVoiceActive(int voice);

    void loadOrdering(const std::array<Node, MAX_GRAPH_NODES> &ordering, int nodes);

    virtual void audioCallback(AudioBuffer Buffer) override;
};

inline Data &Synth::SynthEngine::getDataRef(int voice)
{
    return data;
}

}