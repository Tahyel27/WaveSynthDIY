#pragma once

#include <variant>

#include "PolyphonyManager.hpp"
#include "EffectStack.hpp"
#include "SynthCore/Common.hpp"

struct SetExtRegister
{
    int register_ID;
    float_t val;
};

struct PressNote
{
    int play_ID;
    float_t frequency;
};

struct ReleaseNote
{
    int note_ID;
    float_t frequency;
};

using AudioCommand = std::variant<PressNote, SetExtRegister, ReleaseNote>;
constexpr int COMMAND_QUEUE_LENGTH = 20;
constexpr int NOTE_TRACK_COUNT = Synth::VOICE_COUNT;

struct NoteTracker
{
    bool tracking = false;
    int play_ID;
    int voice_ID;
};

struct AudioCommandQueue
{
    void receive_command(AudioCommand cmd)
    {
        //pushes the command on the queue
    }

    void process_queue(PolyphonyManager &manager)
    {
        //goes through every single command
        //for register commands it just sets the appropriate externa register (register_ID) to the value
        auto ext_register = manager.get_external_register();

        //for note play commands it calls the play_note() method of the manager
        //it keeps track of which play_ID coresponds to which voice_ID received from the manager
        //if there are no free tracking slots (tracking = false) it ignores the command
        //if there is the same play_ID already ignore the command

        //for a release command it reads the play_ID, finds the appropriate voice_ID and releases that voice ID
        //through the polyphony manager, if there is no such play_ID the command is ignored

        //the play_ID search is simply implemented by linearly going through the entire array
    }

private:
    //the queue where incoming commands are stored
    //behaves as a queue but is statically allocated
    std::array<AudioCommand, COMMAND_QUEUE_LENGTH> queue;
    int queue_idx_first = 0;
    int queue_idx_second = 0;

    //tracks which note_ID received from the command corresponds to which voice_ID received from the PolyphonyManager
    std::array<NoteTracker, NOTE_TRACK_COUNT> note_track_array;
};