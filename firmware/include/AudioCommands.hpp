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
    float_t val;
};

// Starts playing frequency Hz through a free voice of the PolyphonyManager.
// play_ID is caller-chosen and later used in ReleaseNote to stop the note;
// duplicate or untrackable play_IDs are ignored.
struct PressNote
{
    int play_ID;
    float_t frequency;
};

// Releases the voice previously pressed with PressNote. note_ID refers to
// the play_ID given at press time; unknown IDs are ignored.
struct ReleaseNote
{
    int note_ID;
    float_t frequency;
};

// A single queued action; one of the command types above.
using AudioCommand = std::variant<PressNote, SetExtRegister, ReleaseNote>;

// Ring-buffer slot count for pending commands. One slot is always kept
// empty so full and empty states are distinguishable, hence 19 commands
// can be buffered at most.
constexpr int COMMAND_QUEUE_LENGTH = 20;

// One tracker per synthesizer voice (Synth::VOICE_COUNT).
constexpr int NOTE_TRACK_COUNT = Synth::VOICE_COUNT;

// Maps a caller-supplied play_ID to the voice_ID handed out by the
// PolyphonyManager while a pressed note is held.
struct NoteTracker
{
    bool tracking = false; // slot free when false
    int play_ID;
    int voice_ID;
};

// Statically allocated FIFO that translates caller-facing note IDs into
// actual synth voices. Not thread-safe by itself: receive_command() is
// meant to be called from the main/core side, process_queue() from the
// audio side under whatever synchronization the application provides.
struct AudioCommandQueue
{
    // Pushes cmd onto the queue. Silently drops it if the queue is full.
    void receive_command(AudioCommand cmd)
    {
        //pushes the command on the queue
        int next = (queue_idx_second + 1) % COMMAND_QUEUE_LENGTH;
        if (next == queue_idx_first)
            return; // queue full, command dropped

        queue[queue_idx_second] = cmd;
        queue_idx_second = next;
    }

    // Drains every queued command in order:
    // - SetExtRegister: writes val into the manager's external register
    //   at register_ID (ignored if out of range).
    // - PressNote: claims a free NoteTracker slot and starts the note via
    //   manager.play_note(); ignored if the play_ID is already tracked,
    //   no slot is free, or no voice is available.
    // - ReleaseNote: linear search for the tracker with matching play_ID,
    //   releases its voice via manager.release_note(); ignored if no
    //   such play_ID exists.
    void process_queue(PolyphonyManager &manager)
    {
        auto ext_register = manager.get_external_register();

        while (queue_idx_first != queue_idx_second)
        {
            AudioCommand &cmd = queue[queue_idx_first];
            queue_idx_first = (queue_idx_first + 1) % COMMAND_QUEUE_LENGTH;

            if (auto *reg = std::get_if<SetExtRegister>(&cmd))
            {
                if (reg->register_ID >= 0 && reg->register_ID < static_cast<int>(REGISTER_SIZE))
                    (*ext_register)[reg->register_ID].f = reg->val;
            }
            else if (auto *press = std::get_if<PressNote>(&cmd))
            {
                NoteTracker *free_slot = nullptr;
                bool already_tracked = false;

                for (auto &tracker : note_track_array)
                {
                    if (!tracker.tracking && !free_slot)
                        free_slot = &tracker;
                    if (tracker.tracking && tracker.play_ID == press->play_ID)
                    {
                        already_tracked = true;
                        break;
                    }
                }

                if (already_tracked || !free_slot)
                    continue; // no free tracking slots or duplicate play_ID

                int voice_ID = manager.play_note(press->frequency);
                if (voice_ID < 0)
                    continue; // no free voices in the manager

                free_slot->tracking = true;
                free_slot->play_ID = press->play_ID;
                free_slot->voice_ID = voice_ID;
            }
            else if (auto *release = std::get_if<ReleaseNote>(&cmd))
            {
                for (auto &tracker : note_track_array)
                {
                    if (tracker.tracking && tracker.play_ID == release->note_ID)
                    {
                        manager.release_note(tracker.voice_ID);
                        tracker.tracking = false;
                        break;
                    }
                }
            }
        }
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
