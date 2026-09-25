#pragma once

#include "stdio.h"

#include "StaticQueue.hpp"
#include "Events.hpp"
#include "AudioStack.hpp"
#include "EncdoderMap.hpp"
#include "MidiParser.hpp"

class UI
{
    //current configuration: 4 encoders exposed
    //button switches between which encoder gets exposed
    using EncoderBank = EncoderBankEntry[4];

    EncoderBank adsr1_bank;  //adsr 1 params
    EncoderBank adsr2_bank;  //adsr 2 params
    EncoderBank wtind_bank;  //wt1-3 wavetable index
    EncoderBank  filt_bank;  //filter 1,2  cutoff,res
    EncoderBank phmod_bank;  //phasemod entry 1-3
    EncoderBank lfofr_bank;  //LFO 1-3 frequency

    EncoderBankEntry * active_encoder_bank;

    const int WAVETABLE_COUNT = 6;

public:
    UI();

    void process_event(Event &event, AudioStack &audio_stack);

    void process_button_press(Event event);
};

UI::UI()
{
    using namespace Synth;
    //setting up the encoder banks with the proper params:
    //ADSR
    const auto attack_encoder = EncoderMapLinear(0.0f, 1.0f, 0.005f);
    const auto decay_encoder = EncoderMapLinear(0.0f, 5.0f, 0.005f, 1.0f);
    const auto sustain_encoder = EncoderMapLinear(0.0f, 1.0f, 0.01f, 0.9f);
    const auto release_encoder = EncoderMapLinear(0.0f, 5.0f, 0.005f, 0.1f);

    adsr1_bank[0] = EncoderBankEntry(attack_encoder, EXT_ADSR1_ATTACK);
    adsr1_bank[1] = EncoderBankEntry(decay_encoder, EXT_ADSR1_DECAY);
    adsr1_bank[2] = EncoderBankEntry(sustain_encoder, EXT_ADSR1_SUSTAIN);
    adsr1_bank[3] = EncoderBankEntry(release_encoder, EXT_ADSR1_RELEASE);

    adsr2_bank[0] = EncoderBankEntry(attack_encoder, EXT_ADSR2_ATTACK);
    adsr2_bank[1] = EncoderBankEntry(decay_encoder, EXT_ADSR2_DECAY);
    adsr2_bank[2] = EncoderBankEntry(sustain_encoder, EXT_ADSR2_SUSTAIN);
    adsr2_bank[3] = EncoderBankEntry(release_encoder, EXT_ADSR2_RELEASE);

    //wavetable index encoder bank
    const auto wtindex_encoder = EncoderMapDiscrete(WAVETABLE_COUNT-1, 2);
    wtind_bank[0] = EncoderBankEntry(wtindex_encoder, EXT_WAVETABLE1_INDEX);
    wtind_bank[1] = EncoderBankEntry(wtindex_encoder, EXT_WAVETABLE2_INDEX);
    wtind_bank[2] = EncoderBankEntry(wtindex_encoder, EXT_WAVETABLE3_INDEX);

    //filter
    const auto filter_f_encoder = EncoderMapLog(50.f, 15000.f, 1000, 50.f);
    const auto filter_r_encoder = EncoderMapLinear(0.0f, 1.0f, 0.01f, 0.9f);
    filt_bank[0] = EncoderBankEntry(filter_f_encoder, EXT_FILT1_CUTOFF);
    filt_bank[1] = EncoderBankEntry(filter_r_encoder, EXT_FILT1_RESONANCE);
    filt_bank[2] = EncoderBankEntry(filter_f_encoder, EXT_FILT2_CUTOFF);
    filt_bank[3] = EncoderBankEntry(filter_r_encoder, EXT_FILT2_RESONANCE);

    //phase modulation
    const auto phimod_encoder = EncoderMapLinear(0.0f, 1.0f, 0.01f, 0.9f);
    phmod_bank[0] = EncoderBankEntry(phimod_encoder, EXT_PHIMOD1_AMOUNT);
    phmod_bank[1] = EncoderBankEntry(phimod_encoder, EXT_PHIMOD2_AMOUNT);
    phmod_bank[2] = EncoderBankEntry(phimod_encoder, EXT_PHIMOD3_AMOUNT);

    const auto lfofreq_encoder = EncoderMapLog(0.1f, 10.f, 500, 1.f);
    lfofr_bank[0] = EncoderBankEntry(lfofreq_encoder, EXT_LFO1_FREQ);
    lfofr_bank[1] = EncoderBankEntry(lfofreq_encoder, EXT_LFO2_FREQ);
    lfofr_bank[2] = EncoderBankEntry(lfofreq_encoder, EXT_LFO3_FREQ);

    //activate first the wavetable index encoder bank:
    active_encoder_bank = &wtind_bank[0];
}   

void UI::process_event(Event &event, AudioStack &audio_stack)
{
    //quick smoke test to see if it works correctly
    if (event.m_type == EventType::ENCODER_TURN)
    {
        auto id = event.get_encoder_turn().encoder_id;
        auto steps = event.get_encoder_turn().change;
        
        auto command_opt = active_encoder_bank[id].turn_and_get_command(steps);
        if (command_opt.has_value())
        {
            auto command = command_opt.value();
            auto setext = std::get<SetExtRegister>(command);

            printf("External register %d will be set to %d\n", setext.register_ID, setext.val.u);

            audio_stack.send_command(command);
        }
    }
    if (event.m_type == EventType::MIDI_MESSAGE)
    {
        auto opt = parse_midi_message(event.get_midi_message());
        if (opt.has_value())
        {
            audio_stack.send_command(opt.value());
        }
    }
    if (event.m_type == EventType::BUTTON_PRESS)
    {
        process_button_press(event);
    }
}

void UI::process_button_press(Event event)
{
    auto button_id = event.get_button_press();

    switch (button_id)
    {
    case 0:  //setting wavetable index
        active_encoder_bank = &wtind_bank[0];
        break;
    case 1: //setting adsr 1
        active_encoder_bank = &adsr1_bank[0];
        break;
    case 2: //setting adsr 2
        active_encoder_bank = &adsr2_bank[0];
        break;
    case 3: //setting filter
        active_encoder_bank = &filt_bank[0];
        break;
    case 4: //setting phase modulation
        active_encoder_bank = &phmod_bank[0];
        break;
    case 5: //setting lfos
        active_encoder_bank = &lfofr_bank[0];
        break;
    default:
        break;
    }
}