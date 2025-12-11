#pragma once

#include <SynthCore/Engine.hpp>
#include <SynthCore/Components.hpp>

class InstrumentManager
{
private:
    static constexpr int INSTRUMENT_COUNT = 3;

    std::array<Synth::Data, INSTRUMENT_COUNT> instrumentsData;
    std::array<Synth::NodeOrder, INSTRUMENT_COUNT> instrumentsOrdering;
    
    unsigned int releaseCounter = 0;
    std::array<unsigned int, Synth::VOICE_COUNT> vReleaseOrder;
    std::array<unsigned int, Synth::VOICE_COUNT> playIDs;
    std::array<int, Synth::VOICE_COUNT> voiceInstrumentMapping = {};

    Synth::SynthEngine * engine;

    int findFreeVoice(int instrument);

public:
    InstrumentManager(Synth::SynthEngine *eng);
    ~InstrumentManager(){};

    void setInstrument(Synth::Data data, Synth::NodeOrder order, int instrument);

    void playFrequency(float frequency, int instrument, int playID);

    void release(int playID);
};