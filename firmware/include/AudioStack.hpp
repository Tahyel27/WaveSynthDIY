#pragma once

#include "PolyphonyManager.hpp"
#include "AudioInterface.hpp"
#include "EffectStack.hpp"
#include "AudioCommands.hpp"

struct NoteState
{
    //the ID of the action that initiated the note press, -1 = free
    int playID;
    //the ID of the voice that is playing the note
    int voiceID;
};

//tracks which voice_id corresponds to which play_id, WARNING: may run out of tracking slots if tracking slots < voices
struct NoteTracker
{
    NoteTracker() 
    {
        for (auto &&e : tracker)
        {
            e = NoteState{-1,-1};
        }
    }
    
    //register a valid pair of playID and voiceID, the voice ID must be acquired from the manager
    //a -1 voice ID will not register
    void register_press(int play_id, int voice_id)
    {
        if (voice_id == -1)
            return;
        
        for (auto &&e: tracker)
        {
            if (e.playID == -1)
            {
                e.playID = play_id;
                e.voiceID = voice_id;
                return;
            }
        }
    }

    //release a current play_id and returns it's corresponding voice id
    //if no voice was using this press, then returns -1
    int register_release(int play_id)
    {
        if (play_id == -1)
            -1;

        for (auto &&e: tracker)
        {
            if (e.playID == play_id)
            {
                int voice_id = e.voiceID;
                e.voiceID = -1; e.playID = -1;
                return voice_id;
            }
        }

        return -1;
    }
private:
    std::array<NoteState, Synth::VOICE_COUNT> tracker;
};

struct AudioStack : public AudioSource
{
    AudioStack(PolyphonyManager &polyphony_manager, EffectStack &effect_stack) 
        : poly_manager(polyphony_manager), fx_stack(effect_stack) {}; 
    
    void audioCallback(AudioBuffer buffer) override 
    {
        poly_manager.render_audio(float_buffer.data());
        fx_stack.apply_effects(float_buffer.data());
        
        for (size_t i = 0; i < Synth::BUFFER_SIZE; i++)
        {
            buffer.write16bit(i, static_cast<int16_t>(Synth::maxamp * float_buffer[i]), AudioBuffer::Mode::MONO);
        }
    }

    //receives and audio command and executes it
    void send_command(AudioCommand cmd)
    {
        if (auto * note_press = std::get_if<PressNote>(&cmd))
        {
            int voice_id = poly_manager.play_note(note_press->frequency, note_press->velocity);
            if (voice_id == -1)
                return;

            note_tracker.register_press(note_press->play_ID, voice_id);
        }
        else if (auto * note_release = std::get_if<ReleaseNote>(&cmd))
        {
            int voice_id = note_tracker.register_release(note_release->play_ID);

            if (voice_id == -1)
                return;

            poly_manager.release_note(voice_id);
        }
        else if (auto * setreg = std::get_if<SetExtRegister>(&cmd))
        {
            poly_manager.get_external_register()->at(setreg->register_ID).f = setreg->val;
        }
    }

    void config_effects(EffectStackConfig fx_conf)
    {
        fx_stack.set_config(fx_conf);
    }
private:
    PolyphonyManager &poly_manager;
    EffectStack &fx_stack;
    alignas(32) std::array<float_t, Synth::BUFFER_SIZE> float_buffer;
    NoteTracker note_tracker;
};