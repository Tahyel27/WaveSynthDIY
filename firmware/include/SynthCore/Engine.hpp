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
    std::array<Instruction, MAX_INSTRUCTION_COUNT> instructions;
    uint16_t instruction_count = 0;

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

    void set_instructions(Instruction * instruct_array, uint16_t count);

    void write_buffer(float_t * buff);

    Context & get_ctx() { return ctx; };

    virtual void audioCallback(AudioBuffer Buffer) override;
};

}