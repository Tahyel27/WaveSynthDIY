#pragma once
//this file holds the information about the conventions between the registers

namespace Synth
{
    //SPECIAL SCALAR REGISTERS
    //these are used to communicate information from the environment
    //to each voice individually (eg. each voice has a different frequency)
    //every patch needs these
    const int SCALAR_REG_FREQUENCY =  0;
    const int SCALAR_REG_VELOCITY  =  1;
    const int SCALAR_REG_GATE      =  2;

    //------------------------------------------------------------------------
    //EXTERNAL REGISTER CONVENTION
    
    //Wavetable oscillators
    const int EXT_WAVETABLE1_INDEX =  0;
    const int EXT_WAVETABLE2_INDEX =  1;
    const int EXT_WAVETABLE3_INDEX =  2;
    const int EXT_WAVETABLE4_INDEX =  3;

    //ADSR:
    //1.
    const int EXT_ADSR1_ATTACK     =  4;
    const int EXT_ADSR1_DECAY      =  5;
    const int EXT_ADSR1_SUSTAIN    =  6;
    const int EXT_ADSR1_RELEASE    =  7;
    //2.
    const int EXT_ADSR2_ATTACK     =  8;
    const int EXT_ADSR2_DECAY      =  9;
    const int EXT_ADSR2_SUSTAIN    = 10;
    const int EXT_ADSR2_RELEASE    = 11;
    //3.
    const int EXT_ADSR3_ATTACK     = 12;
    const int EXT_ADSR3_DECAY      = 13;
    const int EXT_ADSR3_SUSTAIN    = 14;
    const int EXT_ADSR3_RELEASE    = 15;

    //FILTER:
    const int EXT_FILT1_CUTOFF     = 16; 
    const int EXT_FILT1_RESONANCE  = 17;
    const int EXT_FILT2_CUTOFF     = 18;
    const int EXT_FILT2_RESONANCE  = 19;

    //AMPLIFIER:
    const int EXT_AMPL1_GAIN       = 20;
    const int EXT_AMPL2_GAIN       = 21;

    //LFO:
    const int EXT_LFO1_FREQ        = 22;
    const int EXT_LFO2_FREQ        = 23;
    const int EXT_LFO3_FREQ        = 24;

    //PHASE MODULATION ENTRY POINT
    const int EXT_PHIMOD1_AMOUNT   = 25;
    const int EXT_PHIMOD2_AMOUNT   = 26;
    const int EXT_PHIMOD3_AMOUNT   = 27;
};