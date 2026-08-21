#pragma once

#include <array>
#include "SynthCore/Engine.hpp"

using namespace Synth;

enum class VoiceState 
{
    PRESSED,
    RELEASING,
    INACTIVE
};

struct Voice
{
    SynthEngine engine;
    VoiceState state;
    uint32_t release_timer;
};

class PolyphonyManager 
{
    std::array<Voice, VOICE_COUNT> voices;
    int active_voices = 0;

    alignas(32) std::array<float_t, BUFFER_SIZE> master_buffer;

    ScalarRegister * ext_register;

    std::array<Instruction, MAX_INSTRUCTION_COUNT> instructions;
    int instruction_count = 0;

    uint32_t release_wait_time;

    void activate_voice(int ID);

    void deactivate_voice(int ID);

public:
    PolyphonyManager(BufferPool * pool, ShortBufferPool * sb_pool, ScalarRegister * ext) : ext_register(ext), release_wait_time(SPS) {
        for (auto& voice: voices)
        {
            voice = Voice {
                .engine = SynthEngine{ pool, sb_pool, ext },
                .state = VoiceState::INACTIVE,
                .release_timer = 0
            };
        }
    };

    //later refactor this so that the polyphony manager owns the instructions and the engine just holds a pointer
    void set_instructions(Instruction *insts, int count)
    {
        std::copy_n(insts, count, instructions.begin());
        instruction_count = count;
        for (auto& voice: voices)
        {
            voice.engine.set_instructions(instructions.data(), instruction_count);
        }
    }

    void render_audio(float_t * out_buffer);

    int play_note(float_t frequency, float_t velocity = 1.0f);

    void release_note(int ID);

    void stop_note(int ID);

    void set_release_samples(uint32_t release_t);
};

void PolyphonyManager::activate_voice(int ID)
{
    voices[ID].state = VoiceState::PRESSED;
    active_voices++;
}

void PolyphonyManager::deactivate_voice(int ID)
{
    voices[ID].state = VoiceState::INACTIVE;
    active_voices--;
}

void PolyphonyManager::render_audio(float_t * out_buffer)
{
    std::fill_n(out_buffer, BUFFER_SIZE, 0.0);
    
    for (auto& voice : voices)
    {
        if (voice.state != VoiceState::INACTIVE)
        {
            voice.engine.write_buffer(master_buffer.data());
            for (int i = 0; i < BUFFER_SIZE; i++)
            {
                out_buffer[i] += master_buffer[i];
            }

            if (voice.state == VoiceState::RELEASING)
            {
                if (voice.release_timer > release_wait_time)
                {
                    voice.release_timer = 0;
                    voice.state = VoiceState::INACTIVE;
                    active_voices--;
                }
                else
                {
                    voice.release_timer += BUFFER_SIZE;
                }
            }
        }
    }
}

//plays a given frequency and velocity through a voice
//returns the index of the voice playing
//in case of no free voices returns -1
int PolyphonyManager::play_note(float_t frequency, float_t velocity)
{
    for (int i = 0; i < VOICE_COUNT; i++)
    {
        if (voices[i].state == VoiceState::INACTIVE)
        {
            voices[i].engine.get_ctx().set_frequency(frequency);
            voices[i].engine.get_ctx().set_velocity(velocity);
            voices[i].engine.get_ctx().set_gate(true);
            activate_voice(i);
            return i;
        }
    }

    return -1;
}

void PolyphonyManager::release_note(int ID)
{
    voices[ID].engine.get_ctx().set_gate(false);
    voices[ID].state = VoiceState::RELEASING;
}

void PolyphonyManager::stop_note(int ID)
{
    voices[ID].engine.get_ctx().set_gate(false);
    deactivate_voice(ID);
}

void PolyphonyManager::set_release_samples(uint32_t release_t)
{
    release_wait_time = release_t;
}