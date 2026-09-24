#include <SynthCore/Engine.hpp>
#include <SynthCore/Components.hpp>

using namespace Synth;

void SynthEngine::audioCallback(AudioBuffer Buffer)
{    
    processGraph();
    
    output(Buffer);
}

void Synth::SynthEngine::write_buffer(float_t * buff)
{
    processGraph();
    
    std::copy_n(ctx.master_output.data(), BUFFER_SIZE, buff);
}

void Synth::SynthEngine::set_instructions(Instruction *instruct_array, uint16_t count)
{
    std::copy_n(instruct_array, count, instructions.begin());
    instruction_count = count;
}

void Synth::SynthEngine::output(AudioBuffer buffer)
{
    for (size_t i = 0; i < buffer.buffsize; i++)
    {
        buffer.write16bit(i, static_cast<int16_t>(maxamp*ctx.master_output[i]), AudioBuffer::Mode::MONO);
    }
}

void Synth::SynthEngine::processGraph()
{
    // the buffers are chunked, so we need to fill each chunk independetly and supply the correct chunk offset to the node processing
    for (size_t j = 0; j < CHUNKS_PER_BUFFER; j++)
    {
        processChunk(j);
    }
}

void Synth::SynthEngine::processChunk(int chunk)
{
    ctx.current_chunk = chunk;
    
    for (int i = 0; i < instruction_count; i++)
    {
        process_instruction(instructions[i]);
    }
}

int Synth::SynthEngine::process_instruction(Instruction instruction) 
{
    int result = 0;
    switch (instruction.operation)
    {
    case OpCode::ADD:
        op_add(instruction, ctx);
        break;
    case OpCode::MIX:
        op_mix(instruction, ctx);
        break;
    case OpCode::WTOSC:
        op_wtosc(instruction, ctx);
        break;
    case OpCode::SINEOSC:
        op_sineosc(instruction, ctx);
        break;
    case OpCode::AMPL:
        op_ampl(instruction, ctx);
        break;
    case OpCode::ADSR:
        op_adsr(instruction, ctx);
        break;
    case OpCode::SVFILTLP:
        op_svfilt_lp(instruction, ctx);
        break;
    case OpCode::UNISON:
        op_unison(instruction, ctx);
        break;
    case OpCode::LFOSINE:
        op_lfosine(instruction, ctx);
        break;
    case OpCode::LFOTRI:
        op_lfotri(instruction, ctx);
        break;
    case OpCode::LFOSAW:
        op_lfosaw(instruction, ctx);
        break;
    case OpCode::MUL:
        op_mul(instruction, ctx);
        break;
    case OpCode::MUL_SB:
        op_mul_sb(instruction, ctx);
        break;
    case OpCode::ADD_SB:
        op_add_sb(instruction, ctx);
        break;
    case OpCode::MASTER_OUT:
        op_master_out(instruction, ctx);
        break;
    case OpCode::SOFTCLIP:
        op_softclip(instruction, ctx);
        break;
    default:
        break;
    }

    return result;
}

Synth::SynthEngine::SynthEngine()
{

}
