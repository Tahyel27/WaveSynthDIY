#pragma once

#include <variant>

#include <array>

#include "PolyphonyManager.hpp"
#include "EffectStack.hpp"
#include "SynthCore/Common.hpp"

// Commands that control the synthesizer from outside the audio path.
// Queued by non-audio code (e.g. input handling) and drained once per
// buffer by process_queue() on the audio thread.

// Sets external register register_ID (see Synth::ScalarRegister) to val.
// Out-of-range register_ID values are ignored.
struct SetExtRegister
{
    int register_ID;
    float val;

    SetExtRegister(int reg_id, float value) : register_ID(reg_id), val(value) {};
};

// Starts playing frequency Hz through a free voice of the PolyphonyManager.
// play_ID is caller-chosen and later used in ReleaseNote to stop the note;
// duplicate or untrackable play_IDs are ignored.
struct PressNote
{
    int play_ID;
    float frequency;
    float velocity = 1.0;

    PressNote(int play_id, float freq, float vel = 1.0f) 
        : play_ID(play_id), frequency(freq), velocity(vel) {}; 
};

// Releases the voice previously pressed with PressNote. note_ID refers to
// the play_ID given at press time; unknown IDs are ignored.
struct ReleaseNote
{
    int play_ID;
    
    ReleaseNote(int play_id) : play_ID(play_id) {};
};

// A single queued action; one of the command types above.
using AudioCommand = std::variant<PressNote, SetExtRegister, ReleaseNote>;

