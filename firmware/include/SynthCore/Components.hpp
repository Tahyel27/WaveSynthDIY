#pragma once

#include <array>
#include <algorithm>
#include <variant>
#include "SynthCore/Common.hpp"

namespace Synth
{
    enum class OpCode : uint8_t 
    {
        ADD,
        MIX,
        WTOSC,
        SINEOSC,
        AMPL,
        ADSR,
        SVFILTLP,
        UNISON,
        LFOSINE,
        LFOTRI,
        LFOSAW,
        MUL,
        MUL_SB,
        ADD_SB,
        MASTER_OUT
    };

    struct Instruction 
    {
        OpCode operation;
        Operand op1;
        Operand op2;
        Operand op3;
        Operand op4;
        Operand op5;
        Operand op6;
        Operand op7;
        Operand op8;
    };

    enum class ADSRState : uint8_t 
    {
        IDLE,
        ATTACK,
        DECAY,
        SUSTAIN,
        RELEASE
    };

    using VariantType = std::variant<float_t, int, ADSRState>;

    struct ModInput
    {
        int bufID = -1;
        float_t v = 0;
    };

    enum class NodeType {
        WTOSCILLATOR,
        SINEOSCILLATOR,
        SAWOSCILLATOR,
        AMPLIFIER,
        ADSR,
        SVFLP,
        DELAY
    };

    struct DataHolder
    {
        NodeType type;
        std::array<ModInput, 10> data;
    };

    int op_add(Instruction inst, Context &ctx);
    int op_mix(Instruction inst, Context &ctx);
    int op_wtosc(Instruction inst, Context &ctx);
    int op_svfilt_lp(Instruction inst, Context &ctx);
    int op_unison(Instruction inst, Context &ctx);
    int op_ampl(Instruction inst, Context &ctx);
    int op_adsr(Instruction inst, Context &ctx);
    int op_sineosc(Instruction inst, Context &ctx);
    int op_lfosine(Instruction inst, Context &ctx);
    int op_lfotri(Instruction inst, Context &ctx);
    int op_lfosaw(Instruction inst, Context &ctx);
    int op_mul(Instruction inst, Context &ctx);
    int op_mul_sb(Instruction inst, Context &ctx);
    int op_add_sb(Instruction inst, Context &ctx);
    int op_master_out(Instruction inst, Context &ctx);
}