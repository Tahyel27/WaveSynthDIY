#include <SynthCore/Engine.hpp>
#include <SynthCore/Components.hpp>

using namespace Synth;

void SynthEngine::audioCallback(AudioBuffer Buffer)
{    
    processGraph();
    
    output(Buffer);
}

void SynthEngine::loadData(const Data &data_)
{
    data = data_;
};

void Synth::SynthEngine::loadOrdering(const NodeOrder &order)
{
    nodeOrder = order;
}

void Synth::SynthEngine::set_instructions(Instruction *instruct_array, uint16_t count)
{
    std::copy_n(instruct_array, count, instructions.begin());
    instruction_count = count;
}

std::tuple<Data &, NodeOrder &> Synth::SynthEngine::getDataForVoiceRef(int voice)
{
    return std::tuple<Data &, NodeOrder &>(data,nodeOrder);
}


void Synth::SynthEngine::output(AudioBuffer buffer)
{
    for (size_t i = 0; i < buffer.buffsize; i++)
    {
        buffer.write16bit(i, static_cast<int16_t>(maxamp*output_buffer[i]), AudioBuffer::Mode::MONO);
    }
}

void Synth::SynthEngine::processGraph()
{
    // the buffers are chunked, so we need to fill each chunk independetly and supply the correct chunk offset to the node processing
    for (size_t j = 0; j < CHUNKS_PER_BUFFER; j++)
    {
        processChunk(j);

        std::copy_n(ctx.bufferPool->getBuffer(0), CHUNK_SIZE, &output_buffer[j*CHUNK_SIZE]);
    }
}

void Synth::SynthEngine::processChunk(int chunk)
{
    ctx.bufferPool->wipeBuffers();
    //we iterate over the operations in the queue
    /*for (size_t i = 0; i < nodeOrder.nodeCount; i++)
    {
        //we send the node to processing
        //we have to send the NodeData array of our current voice, our current output buffer(as a pointer, we can always do this), and the buffer pool
        //the final output buffer pointer will wary depending on the chunk
        float_t * outbuffer;
        if (nodeOrder.data[i].outputBuffer != -1)
        {
            outbuffer = ctx.bufferPool->getBuffer(nodeOrder.data[i].outputBuffer);
        }
        else
        {
            outbuffer = &output_buffer[CHUNK_SIZE * chunk];
        }
        
        processNode(nodeOrder.data[i].type, nodeOrder.data[i].dataIndex, data, outbuffer, ctx.bufferPool);
    }*/

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
        break;
    case OpCode::AMPL:
        break;
    case OpCode::ADSR:
        break;
    case OpCode::SVFILTLP:
        break;
    default:
        break;
    }

    return result;
}

Synth::SynthEngine::SynthEngine()
{

}
