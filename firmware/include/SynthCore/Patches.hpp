#pragma once

#include <SynthCore/Components.hpp>

namespace Synth
{
    void createSimpleWTPatch(Data &data, NodeOrder &order, int wtindex)
    {
        order.nodeCount = 1;
        
        WTOscData osc;
        osc.wtIndex = wtindex;
        osc.unison = 0;
        osc.detune = ModInput{-1, 0.3};
        osc.phaseDistMod = ModInput{-1, 0};
        osc.phaseDistMod = ModInput{-1, 0};
        osc.freq = ModInput{-1, 250};
        osc.morph = ModInput{-1, 0};

        data.WTOscArr[0] = osc;
        order.data[0].type = NodeType::WTOSCILLATOR;
        order.data[0].dataIndex = 0;
        order.data[0].outputBuffer = -1;
    }

    void createSimpleWTPatchWithADSR(Data &data, NodeOrder &order)
    {
        order.nodeCount = 3;
        
        WTOscData osc;
        osc.wtIndex = 1;
        osc.unison = 0;
        osc.detune = ModInput{-1, 0.3};
        osc.phaseDistMod = ModInput{-1, 0};
        osc.phaseDistMod = ModInput{-1, 0};
        osc.freq = ModInput{-1, 200};
        osc.morph = ModInput{-1, 0};

        data.WTOscArr[0] = osc;
        order.data[0].type = NodeType::WTOSCILLATOR;
        order.data[0].dataIndex = 0;
        order.data[0].outputBuffer = 0;

        auto adsr = ADSRData();
        adsr.state = ADSRData::State::ATTACK;
        adsr.sustain = -0.5;

        data.ADSRArr[0] = adsr;
        order.data[1].type = NodeType::ADSR;
        order.data[1].dataIndex = 0;
        order.data[1].outputBuffer = 1;

        data.AmplifierArr[0] = AmplifierData{{0, 0.0}, {1, 0.5}};
        order.data[2].type = NodeType::AMPLIFIER;
        order.data[2].dataIndex = 0;
        order.data[2].outputBuffer = -1;
    }

    void createWTPatchwithFilter(Data &data, NodeOrder &order)
    {
        order.nodeCount = 2;
        
        WTOscData osc;
        osc.wtIndex = 1;
        osc.unison = 1;
        osc.detune = ModInput{-1, 1};
        osc.phaseDistMod = ModInput{-1, 0};
        osc.phaseDistMod = ModInput{-1, 0};
        osc.freq = ModInput{-1, 200};
        osc.morph = ModInput{-1, 0};

        data.WTOscArr[0] = osc;
        order.data[0].type = NodeType::WTOSCILLATOR;
        order.data[0].dataIndex = 0;
        order.data[0].outputBuffer = 0;

        SVFData filt;
        filt.input = ModInput{0, 0};
        filt.Q = 0.1;
        filt.fcut = 2500;

        data.SVFArr[0] = filt;
        order.data[1].type = NodeType::SVFLP;
        order.data[1].dataIndex = 0;
        order.data[1].outputBuffer = -1;
    }

    void createFMWTPatch(Data &data, NodeOrder &order, float freq)
    {
        order.nodeCount = 2;

        SineOscData osc2;
        osc2.freq.v = freq;

        data.SineOscArr[0] = osc2;
        order.data[0].type = NodeType::SINEOSCILLATOR;
        order.data[0].dataIndex = 0;
        order.data[0].outputBuffer = 0;

        WTOscData osc;
        osc.wtIndex = 0;
        osc.unison = 1;
        osc.phaseDistort.bufID = 0;
        osc.phaseDistMod.v = 0.4;
        osc.freq.v = freq;
        osc.morph.v = 0;

        data.WTOscArr[0] = osc;
        order.data[1].type = NodeType::WTOSCILLATOR;
        order.data[1].dataIndex = 0;
        order.data[1].outputBuffer = -1;
    }

    void createFMWTPatchWithADSR(Data &data, NodeOrder &order);

    void createPatchAlgo1(Data &data, NodeOrder &order, float_t freq, float_t fcut, float_t fmod)
    {
        order.nodeCount = 4;

        SineOscData sine1;
        sine1.freq.v = freq;
        data.SineOscArr[0] = sine1;

        order.data[0].type = NodeType::SINEOSCILLATOR;
        order.data[0].dataIndex = 0;
        order.data[0].outputBuffer = 0;

        WTOscData wtosc;
        wtosc.wtIndex = 1;
        wtosc.unison = 3;
        wtosc.detune.v = 0.6;
        wtosc.phaseDistort.bufID = 0;
        wtosc.phaseDistMod.v = fmod;
        wtosc.freq.v = freq;

        data.WTOscArr[0] = wtosc;
        order.data[1].type = NodeType::WTOSCILLATOR;
        order.data[1].outputBuffer = 1;
        order.data[1].dataIndex = 0;

        SineOscData sine2;
        sine2.freq.v = 1;
        data.SineOscArr[1] = sine2;

        order.data[2].type = NodeType::SINEOSCILLATOR;
        order.data[2].outputBuffer = 2;
        order.data[2].dataIndex = 1;

        SVFData filt;
        filt.fcut = fcut;
        filt.Q = 0.1;
        filt.fenv = 400;
        filt.input.bufID = 1;
        filt.modulation.bufID = 2;
        data.SVFArr[0] = filt;

        order.data[3].type = NodeType::SVFLP;
        order.data[3].outputBuffer = -1;
        order.data[3].dataIndex = 0;
    }
} // namespace Synth
