#include <SynthCore/Engine.hpp>
#include <SynthCore/Components.hpp>

using namespace Synth;

void SynthEngine::audioCallback(AudioBuffer Buffer)
{    
    processGraph();
    
    outputFromVoices(Buffer);
}

void Synth::SynthEngine::initOneSineTest()
{
    activeVoices.reset();
    activeVoices[0] = true;
    nodeCount = 1;
    voiceData[0].SineOscArr[0] = SineOscData{0, {-1, 100}, {-1, 0}, {-1, 0}};  
    nodeOrder[0].type = NodeType::SINEOSCILLATOR;
    nodeOrder[0].dataIndex = 0;
    nodeOrder[0].outputBuffer = -1; 
}

void Synth::SynthEngine::initSineAndAmpTest()
{
    activeVoices.reset();
    activeVoices[0] = true;
    nodeCount = 2;
    voiceData[0].SineOscArr[0] = SineOscData{0, {-1, 100}, {-1, 0}, {-1, 0}};
    nodeOrder[0].type = NodeType::SINEOSCILLATOR;
    nodeOrder[0].dataIndex = 0;
    nodeOrder[0].outputBuffer = 0;
    voiceData[0].AmplifierArr[0] = AmplifierData{{0,0.0},{-1,0.5}};
    nodeOrder[1].type = NodeType::AMPLIFIER;
    nodeOrder[1].dataIndex = 0;
    nodeOrder[1].outputBuffer = -1;
}
// copies in data into the internal data representation, assigns buffers from pool to indicies and assigns to 6 voices
void Synth::SynthEngine::initSinModAmpTest()
{
    activeVoices.reset();
    activeVoices[0] = true;
    nodeCount = 3;
    
    voiceData[0].SineOscArr[0] = SineOscData{0, {-1, 1000}, {-1, 0}, {-1, 0}};
    nodeOrder[0].type = NodeType::SINEOSCILLATOR;
    nodeOrder[0].dataIndex = 0;
    nodeOrder[0].outputBuffer = 0;
    
    voiceData[0].SineOscArr[1] = SineOscData{0, {-1, 80}, {-1, 0}, {-1, 0}};
    nodeOrder[1].type = NodeType::SINEOSCILLATOR;
    nodeOrder[1].dataIndex = 1;
    nodeOrder[1].outputBuffer = 1;

    voiceData[0].AmplifierArr[0] = AmplifierData{{0, 0.0}, {1, 0.5}};
    nodeOrder[2].type = NodeType::AMPLIFIER;
    nodeOrder[2].dataIndex = 0;
    nodeOrder[2].outputBuffer = -1;
}

void Synth::SynthEngine::initSinFMModTest()
{
    activeVoices.reset();
    activeVoices[0] = true;
    nodeCount = 3;

    voiceData[0].SineOscArr[0] = SineOscData{0, {-1, 200}, {-1, 0}, {-1, 0}};
    nodeOrder[0].type = NodeType::SINEOSCILLATOR;
    nodeOrder[0].dataIndex = 0;
    nodeOrder[0].outputBuffer = 0;

    voiceData[0].SineOscArr[1] = SineOscData{0, {-1, 100}, {0, 0}, {-1, 0.2}};
    nodeOrder[1].type = NodeType::SINEOSCILLATOR;
    nodeOrder[1].dataIndex = 1;
    nodeOrder[1].outputBuffer = -1;
}

void Synth::SynthEngine::initWTTest()
{
    activeVoices.reset();
    activeVoices[0] = true;
    nodeCount = 1;

    WTOscData osc;
    osc.wtIndex = 3;
    osc.unison = 0;
    osc.detune = ModInput{-1,0.3};
    osc.phaseDistMod = ModInput{-1,0};
    osc.phaseDistMod = ModInput{-1,0};
    osc.freq = ModInput{-1,90};
    osc.morph = ModInput{-1,0};

    voiceData[0].WTOscArr[0] = osc;
    nodeOrder[0].type = NodeType::WTOSCILLATOR;
    nodeOrder[0].dataIndex = 0;
    nodeOrder[0].outputBuffer = -1;
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
    nodeOrder = ordering;
    nodeCount = nodes;
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
                tmp[j] += static_cast<float>(maxamp)*output[j];
            }
        }
    }

    for (size_t i = 0; i < buffer.buffsize; i++)
    {
        buffer.write16bit(i, static_cast<int16_t>(tmp[i]), AudioBuffer::Mode::MONO);
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
    //we iterate over the operations in the queue
    for (size_t i = 0; i < nodeCount; i++)
    {
        //we send the node to processing
        //we have to send the NodeData array of our current voice, our current output buffer(as a pointer, we can always do this), and the buffer pool
        //the final output buffer pointer will wary depending on the chunk
        float_t * outbuffer;
        if (nodeOrder[i].outputBuffer != -1)
        {
            outbuffer = bufferPool.getBuffer(nodeOrder[i].outputBuffer);
        }
        else
        {
            outbuffer = &voiceOutputs.get(voice)[CHUNK_SIZE * chunk];
        }
        
        processNode(nodeOrder[i].type, nodeOrder[i].dataIndex, voiceData[voice], outbuffer, &bufferPool);
    }
    
}

void Synth::processNode(NodeType type, int nodeID, Data &data, float_t *outbuffer, BufferPool *buffers)
{
    switch (type)
    {
    case NodeType::WTOSCILLATOR:
        processWTOsc(&data.WTOscArr[nodeID], buffers, outbuffer);
        break;
    case NodeType::AMPLIFIER:
        processAmplifier(data.AmplifierArr[nodeID], buffers, outbuffer);
        break;
    case NodeType::SINEOSCILLATOR:
        processSineOsc(&data.SineOscArr[nodeID], buffers, outbuffer);
        break;
    default:
        break;
    }
}
