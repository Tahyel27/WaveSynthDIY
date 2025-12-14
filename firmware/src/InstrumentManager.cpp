#include "InstrumentManager.hpp"

int InstrumentManager::findFreeVoice(int instrument)
{
    int earliestReleased = -1;
    for (size_t i = 0; i < Synth::VOICE_COUNT; i++)
    {
        if ((voiceInstrumentMapping[i] == instrument) && !engine->isVoiceActive(i))
        {
            if (earliestReleased == -1)
            {
                earliestReleased = i;
            }
            else if (vReleaseOrder[i] < vReleaseOrder[earliestReleased])
            {
                earliestReleased = i;
            }
        }
    }

    return earliestReleased;
}

InstrumentManager::InstrumentManager(Synth::SynthEngine *eng)
{
    engine = eng;
}

void InstrumentManager::setInstrument(Synth::Data data, Synth::NodeOrder order, int instrument)
{
    if (instrument < INSTRUMENT_COUNT)
    {
        instrumentsData[instrument] = data;
        instrumentsOrdering[instrument] = order;
    }
}

void InstrumentManager::playFrequency(float frequency, int instrument, int playID)
{
    if (instrument >= INSTRUMENT_COUNT)
    {
        return;
    }

    int targetVoice = findFreeVoice(instrument);
    if (targetVoice == -1)
    {
        return;
    }

    auto [data, ord] = engine->getDataForVoiceRef(targetVoice);
    data = instrumentsData[instrument];
    ord = instrumentsOrdering[instrument];
    data.WTOscArr[0].freq.v = frequency;

    engine->startVoice(targetVoice);
    playIDs[targetVoice] = playID;
}

void InstrumentManager::release(int playID)
{
    for (size_t i = 0; i < Synth::VOICE_COUNT; i++)
    {
        if (playIDs[i] == playID)
        {
            engine->stopVoice(i);
            releaseCounter++;
            vReleaseOrder[i] = releaseCounter;
            playIDs[i] = -1;
        }
        
    }
    
}
