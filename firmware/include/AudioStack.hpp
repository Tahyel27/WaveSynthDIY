#pragma once

#include "PolyphonyManager.hpp"
#include "AudioInterface.hpp"
#include "EffectStack.hpp"

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
private:
    PolyphonyManager &poly_manager;
    EffectStack &fx_stack;
    alignas(32) std::array<float_t, Synth::BUFFER_SIZE> float_buffer;
};