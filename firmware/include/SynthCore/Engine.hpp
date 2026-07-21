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


class SynthEngine : public AudioSource
{
private:
    BufferPool * bufferPool;    

    NodeOrder nodeOrder;

    int nodeCount = 0;

    Data data;

    std::array<float_t, BUFFER_SIZE> output_buffer;

    void output(AudioBuffer buffer);

    void processGraph();

    void processChunk(int chunk);

 public:
    SynthEngine(/* args */);
    SynthEngine(BufferPool * pool) : bufferPool(pool) {};
    ~SynthEngine(){};

    void loadData(const Data &data_);

    std::tuple<Data&, NodeOrder&> getDataForVoiceRef(int voice);

    inline Data& getDataRef(int voice);

    void loadOrdering(const NodeOrder &order);

    virtual void audioCallback(AudioBuffer Buffer) override;
};

inline Data &Synth::SynthEngine::getDataRef(int voice)
{
    return data;
}

}