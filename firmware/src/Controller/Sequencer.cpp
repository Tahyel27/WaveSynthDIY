#include "Controller/Sequencer.hpp"

float getFrequencyFromScale(int note, int scale, int scale_type)
{
    std::array<float, 8> cMajorScale = {
        130.81f, // C3
        146.83f, // D3
        164.81f, // E3
        174.61f, // F3
        196.00f, // G3
        220.00f, // A3
        246.94f, // B3
        261.63f  // C4
    };

    return cMajorScale[note];
}

Sequencer::Sequencer(Synth::SynthEngine *eng) : manager(eng)
{
    data.tracks[0].data = {0, 0, 2, 4, 0, 0, 3, 1};
    data.tracks[1].data = {5, 7, 5, 7, 5, 7, 5, 7};
    data.activeTracks.reset();
    data.activeTracks[0] = true;
    data.activeTracks[1] = true;
}

void Sequencer::tick()
{
    data.counter++;
    if (data.counter == data.counterMax)
    {
        data.counter = 0;
        data.index++;
        if (data.index == data.tracks[0].data.size())
        {
            data.index = 0;
        }

        for (size_t i = 0; i < data.TRACK_COUNT; i++)
        {
            if (data.activeTracks[i])
            {
                manager.release(i);
                float f = getFrequencyFromScale(data.tracks[i][data.index], data.tracks[i].scale, data.tracks[i].scale_type);
                manager.playFrequency(f, 0, i);
            }
        }
    }
}

InstrumentManager *Sequencer::getManager()
{
    return &manager;
}
