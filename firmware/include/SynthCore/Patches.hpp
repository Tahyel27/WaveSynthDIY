#include <SynthCore/Components.hpp>

namespace Synth
{
    void createSimpleWTPatch(Data &data, NodeOrder &order)
    {
        order.nodeCount = 1;
        
        WTOscData osc;
        osc.wtIndex = 3;
        osc.unison = 0;
        osc.detune = ModInput{-1, 0.3};
        osc.phaseDistMod = ModInput{-1, 0};
        osc.phaseDistMod = ModInput{-1, 0};
        osc.freq = ModInput{-1, 90};
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
        osc.freq = ModInput{-1, 90};
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
        filt.Q = 1;

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
        osc.phaseDistMod.v = 0.8;
        osc.freq.v = freq;
        osc.morph.v = 0;

        data.WTOscArr[0] = osc;
        order.data[1].type = NodeType::WTOSCILLATOR;
        order.data[1].dataIndex = 0;
        order.data[1].outputBuffer = -1;
    }

    void createFMWTPatchWithADSR(Data &data, NodeOrder &order);
} // namespace Synth
