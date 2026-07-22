#pragma once

#include <SynthCore/Components.hpp>

namespace Synth
{
    struct PatchDef {
        const Instruction* instructions;
        uint16_t count;
    };

    inline PatchDef createSimpleWTPatch(int wtindex)
    {
        static Instruction instructions[] = {
            {
                OpCode::WTOSC, 
                Operand::Immediate_f(250.), 
                Operand::ScalarReg(0), 
                Operand::Immediate_f(0.), 
                Operand::Immediate_u(0), // will be replaced
                Operand::Immediate_f(0.), 
                Operand::BufferReg(0)
            }
        };
        instructions[0].op5.value.u = wtindex;
        return {instructions, 1};
    }

    inline PatchDef createSimpleWTPatchWithADSR()
    {
        static Instruction instructions[] = {
            {
                OpCode::WTOSC, 
                Operand::Immediate_f(200.), 
                Operand::ScalarReg(0), 
                Operand::Immediate_f(0.), 
                Operand::Immediate_u(1), 
                Operand::Immediate_f(0.), 
                Operand::BufferReg(1)
            },
            {
                OpCode::ADSR,
                Operand::Immediate_f(1.0f), 
                Operand::ScalarReg(1), 
                Operand::ScalarReg(2), 
                Operand::Immediate_f(0.05f), 
                Operand::Immediate_f(0.1f), 
                Operand::Immediate_f(0.5f), // sustain
                Operand::Immediate_f(0.0f), 
                Operand::BufferReg(2)
            },
            {
                OpCode::AMPL,
                Operand::BufferReg(1), 
                Operand::BufferReg(2), 
                Operand::BufferReg(0)
            }
        };
        return {instructions, 3};
    }

    inline PatchDef createWTPatchwithFilter()
    {
        static Instruction instructions[] = {
            {
                OpCode::WTOSC, 
                Operand::Immediate_f(200.), 
                Operand::ScalarReg(0), 
                Operand::Immediate_f(0.), 
                Operand::Immediate_u(1), 
                Operand::Immediate_f(0.), 
                Operand::BufferReg(1)
            },
            {
                OpCode::SVFILTLP,
                Operand::ScalarReg(1), 
                Operand::ScalarReg(2), 
                Operand::BufferReg(1), 
                Operand::Immediate_f(2500.), 
                Operand::Immediate_f(0.1f), 
                Operand::BufferReg(0)
            }
        };
        return {instructions, 2};
    }

    // --- Complex patches left for manual porting ---

    /*
    inline PatchDef createFMWTPatch(float freq)
    {
        // TODO: Port to VM instructions
        return {nullptr, 0};
    }

    inline PatchDef createFMWTPatchWithADSR()
    {
        // TODO: Port to VM instructions
        return {nullptr, 0};
    }

    inline PatchDef createPatchAlgo1(float_t freq, float_t fcut, float_t fmod)
    {
        // TODO: Port to VM instructions
        return {nullptr, 0};
    }

    inline PatchDef createPatchAlgo2()
    {
        // TODO: Port to VM instructions
        return {nullptr, 0};
    }
    */
} // namespace Synth
