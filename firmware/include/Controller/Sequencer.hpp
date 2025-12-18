#pragma once

#include <array>
#include <SynthCore/Engine.hpp>
#include "InstrumentManager.hpp"

template<size_t length>
struct NoteTrack
{
    std::array<int, length> data;
    
    int scale = 0; //C - 0
    int scale_type = 0; //major - 0
    int octave = 0; //C4-C8

    int & operator[](int i)
    {
        return data[i];
    }

    int & operator[](int i) const
    {
        return data[i];
    }
};

struct SequencerData
{
    static constexpr int TRACK_COUNT = 3;
    std::array<NoteTrack<8>, TRACK_COUNT> tracks;
    std::bitset<TRACK_COUNT> activeTracks;
    int index = 0;
    uint32_t counter = 0;
    uint32_t counterMax = 25;
};

class Sequencer
{
private:
    InstrumentManager manager;

    SequencerData data;
public:
    Sequencer(Synth::SynthEngine * eng);
    ~Sequencer(){};

    void tick();

    InstrumentManager * getManager();
    
};

