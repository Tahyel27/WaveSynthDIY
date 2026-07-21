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
    }
}

void Synth::SynthEngine::processChunk(int chunk)
{
    bufferPool.wipeBuffers();
    //we iterate over the operations in the queue
    for (size_t i = 0; i < nodeOrder.nodeCount; i++)
    {
        //we send the node to processing
        //we have to send the NodeData array of our current voice, our current output buffer(as a pointer, we can always do this), and the buffer pool
        //the final output buffer pointer will wary depending on the chunk
        float_t * outbuffer;
        if (nodeOrder.data[i].outputBuffer != -1)
        {
            outbuffer = bufferPool.getBuffer(nodeOrder.data[i].outputBuffer);
        }
        else
        {
            outbuffer = &output_buffer[CHUNK_SIZE * chunk];
        }
        
        processNode(nodeOrder.data[i].type, nodeOrder.data[i].dataIndex, data, outbuffer, &bufferPool);
    }
    
}

Synth::SynthEngine::SynthEngine()
{

}
