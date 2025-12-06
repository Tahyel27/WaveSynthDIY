#include <array>
#include <bitset>
#include <algorithm>
#include "AudioInterface.hpp"
#include "Components.hpp"
#include "SynthCore/Common.hpp"

namespace Synth
{
    
struct VoiceBuffers
{
    std::array<float_t, BUFFER_SIZE*VOICE_COUNT> data;

    inline float_t * get(int voice)
    {
        return &data[BUFFER_SIZE * voice];
    }
};

struct Node
{
    NodeType type;
    int dataIndex;
    int outputBuffer;
};

//defines an array of component data for each voice, sicne in different voices oscillators could be in a different phase for example
struct Data
{
    static constexpr int WTOscNum = 2;
    std::array<WTOscData, WTOscNum> WTOscArr;

    static constexpr int OSCNum = 4;
    std::array<SineOscData, OSCNum> SineOscArr;
    std::array<SawOscData,  OSCNum> SawOscArr;

    static constexpr int AmplifierNum = 5;
    std::array<AmplifierData, AmplifierNum> AmplifierArr;

    static constexpr int ADSRNum = 5;
    std::array<ADSRData, ADSRNum> ADSRArr;
};

void processNode(NodeType type, int nodeID, Data &data, float_t *outbuffer, BufferPool *buffers);

class SynthEngine : public AudioSource
{
private:
    BufferPool bufferPool;    

    std::array<Node, MAX_GRAPH_NODES> nodeOrder;

    int nodeCount = 0;

    std::array<Data, VOICE_COUNT> voiceData;

    VoiceBuffers voiceOutputs;

    std::bitset<VOICE_COUNT> activeVoices;

    void outputFromVoices(AudioBuffer buffer);

    void processGraph();

    void processChunk(int chunk, int voice);

 public:
    SynthEngine(/* args */){};
    ~SynthEngine(){};

    void initOneSineTest();

    void initSineAndAmpTest();

    void initSinModAmpTest();

    void initSinFMModTest();

    void initWTTest();

    void initADSRTest();

    void loadData(const Data &data_);

    void loadOrdering(const std::array<Node, MAX_GRAPH_NODES> &ordering, int nodes);

    virtual void audioCallback(AudioBuffer Buffer) override;
};

}