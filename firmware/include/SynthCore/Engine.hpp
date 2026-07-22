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

    Context ctx;

    NodeOrder nodeOrder;

    std::array<Instruction, MAX_INSTRUCTION_COUNT> instructions;
    uint16_t instruction_count;

    int nodeCount = 0;

    Data data;

    std::array<float_t, BUFFER_SIZE> output_buffer;

    void output(AudioBuffer buffer);

    void processGraph();

    void processChunk(int chunk);

    int process_instruction(Instruction instruction);

 public:
    SynthEngine(/* args */);
    SynthEngine(BufferPool * pool, ShortBufferPool * short_pool, ScalarRegister * ext_reg)
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

    void set_instructions(Instruction * instruct_array, uint16_t count);

    virtual void audioCallback(AudioBuffer Buffer) override;
};

inline Data &Synth::SynthEngine::getDataRef(int voice)
{
    return data;
}

}