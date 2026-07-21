#include <SynthCore/Engine.hpp>
#include <SynthCore/Components.hpp>

using namespace Synth;

void SynthEngine::audioCallback(AudioBuffer Buffer)
{    
    processGraph();
    
    outputFromVoices(Buffer);
}

void SynthEngine::loadData(const Data &data_)
{
    data = data_;
};

void Synth::SynthEngine::loadOrdering(const std::array<Node, MAX_GRAPH_NODES> &ordering, int nodes)
{
    nodeOrder.data = ordering;
    nodeCount = nodes;
}

void Synth::SynthEngine::loadVoiceData(const Synth::Data &data_, const Synth::NodeOrder &order_, int voice)
{
    if (voice >= VOICE_COUNT)
    {
        return;
    }

    data = data_;
    nodeOrder = order_;
}

std::tuple<Data &, NodeOrder &> Synth::SynthEngine::getDataForVoiceRef(int voice)
{
    return std::tuple<Data &, NodeOrder &>(data,nodeOrder);
}

void Synth::SynthEngine::startVoice(int voice)
{
    if (voice >= VOICE_COUNT)
    {
        return;
    }
    activeVoices[voice] = true;
}

void Synth::SynthEngine::stopVoice(int voice)
{
    if (voice >= VOICE_COUNT)
    {
        return;
    }
    activeVoices[voice] = false;
}

bool Synth::SynthEngine::isVoiceActive(int voice)
{
    return activeVoices[voice];
}

void Synth::SynthEngine::outputFromVoices(AudioBuffer buffer)
{
    float_t gain = 1.0f / sqrtf(static_cast<float>(activeVoices.count()));
    
    std::array<float, BUFFER_SIZE> tmp{};
    for (size_t i = 0; i < VOICE_COUNT; i++)
    {
        if (activeVoices[i])
        {
            float_t * output = output_buffer.data();
            for (size_t j = 0; j < BUFFER_SIZE; j++)
            {
                tmp[j] += gain * output[j];
            }
        }
    }

    for (size_t i = 0; i < buffer.buffsize; i++)
    {
        buffer.write16bit(i, static_cast<int16_t>(maxamp*tmp[i]), AudioBuffer::Mode::MONO);
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
