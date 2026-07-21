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

struct Context
{
    std::array<VariantType, REGISTER_SIZE> variant_reg;
    std::array<float_t, REGISTER_SIZE> scalar_reg;

    ShortBufferPool *short_buf_pool;
    BufferPool *bufferPool;
    ScalarRegister *external_register;
};

class SynthEngine : public AudioSource
{
private:

    Context ctx;

    NodeOrder nodeOrder;

    int nodeCount = 0;

    Data data;

    std::array<float_t, BUFFER_SIZE> output_buffer;

    void output(AudioBuffer buffer);

    void processGraph();

    void processChunk(int chunk);

 public:
    SynthEngine(/* args */);
    SynthEngine(BufferPool * pool, ShortBufferPool * short_pool ,ScalarRegister * ext_reg)
    {
        ctx.bufferPool = pool;
        ctx.short_buf_pool = short_pool;
        ctx.external_register = ext_reg;
    }
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