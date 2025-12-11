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
    for (size_t i = 0; i < VOICE_COUNT; i++)
    {
        voiceData[i] = data_;
    }
};

void Synth::SynthEngine::loadOrdering(const std::array<Node, MAX_GRAPH_NODES> &ordering, int nodes)
{
    nodeOrderArray[0].data = ordering;
    nodeCount = nodes;
}

void Synth::SynthEngine::loadVoiceData(const Synth::Data &data_, const Synth::NodeOrder &order_, int voice)
{
    if (voice >= VOICE_COUNT)
    {
        return;
    }

    voiceData[voice] = data_;
    nodeOrderArray[voice] = order_;
}

std::tuple<Data &, NodeOrder &> Synth::SynthEngine::getDataForVoiceRef(int voice)
{
    return std::tuple<Data &, NodeOrder &>(voiceData[voice],nodeOrderArray[voice]);
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

void Synth::SynthEngine::setDelay(bool state)
{
    useDelay = state;
}

void Synth::SynthEngine::outputFromVoices(AudioBuffer buffer)
{
    std::array<float, BUFFER_SIZE> tmp{};
    for (size_t i = 0; i < VOICE_COUNT; i++)
    {
        if (activeVoices[i])
        {
            float_t * output = voiceOutputs.get(i);
            for (size_t j = 0; j < BUFFER_SIZE; j++)
            {
                tmp[j] += output[j];
            }
        }
    }

    if (useDelay)
    {
        globalDelay(&delayLine, tmp.begin(), tmp.begin(), tmp.size());
    }

    for (size_t i = 0; i < buffer.buffsize; i++)
    {
        buffer.write16bit(i, static_cast<int16_t>(maxamp*tmp[i]), AudioBuffer::Mode::MONO);
    }
}

void Synth::SynthEngine::processGraph()
{
    for (size_t i = 0; i < VOICE_COUNT; i++)
    {
        if (activeVoices[i])
        {
            //here we are going to fill the audio buffer for each of the voices
            //the buffers are chunked, so we need to fill each chunk independetly and supply the correct chunk offset to the node processing
            for (size_t j = 0; j < CHUNKS_PER_BUFFER; j++)
            {
                processChunk(j, i);
            }
        }   
    }
}

void Synth::SynthEngine::processChunk(int chunk, int voice)
{
    bufferPool.wipeBuffers();
    //we iterate over the operations in the queue
    for (size_t i = 0; i < nodeOrderArray[voice].nodeCount; i++)
    {
        //we send the node to processing
        //we have to send the NodeData array of our current voice, our current output buffer(as a pointer, we can always do this), and the buffer pool
        //the final output buffer pointer will wary depending on the chunk
        float_t * outbuffer;
        if (nodeOrderArray[voice].data[i].outputBuffer != -1)
        {
            outbuffer = bufferPool.getBuffer(nodeOrderArray[voice].data[i].outputBuffer);
        }
        else
        {
            outbuffer = &voiceOutputs.get(voice)[CHUNK_SIZE * chunk];
        }
        
        processNode(nodeOrderArray[voice].data[i].type, nodeOrderArray[voice].data[i].dataIndex, voiceData[voice], outbuffer, &bufferPool);
    }
    
}

