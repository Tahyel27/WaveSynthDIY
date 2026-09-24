#pragma once

#include <SynthCore/Components.hpp>

namespace Synth
{
    struct PatchDef {
        Instruction* instructions;
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
            },
            {
                OpCode::MASTER_OUT,
                Operand::BufferReg(0)
            }
        };
        instructions[0].op5.value.u = wtindex;
        return {instructions, 2};
    }

    inline PatchDef createSimpleWTPatchWithADSR()
    {
        static Instruction instructions[] = {
            {
                OpCode::WTOSC, 
                Operand::ScalarReg(0), 
                Operand::ScalarReg(10), 
                Operand::Immediate_f(0.), 
                Operand::Immediate_u(1), 
                Operand::Immediate_f(0.), 
                Operand::BufferReg(1)
            },
            {
                OpCode::ADSR,
                Operand::ScalarReg(2), 
                Operand::ScalarReg(3), 
                Operand::ScalarReg(4), 
                Operand::Immediate_f(0.1f), 
                Operand::Immediate_f(0.3f), 
                Operand::Immediate_f(0.6f), // sustain
                Operand::Immediate_f(0.5f), 
                Operand::BufferReg(2)
            },
            {
                OpCode::AMPL,
                Operand::BufferReg(1), 
                Operand::BufferReg(2), 
                Operand::BufferReg(0)
            },
            {
                OpCode::MASTER_OUT,
                Operand::BufferReg(0)
            }
        };
        return {instructions, 4};
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
            },
            {
                OpCode::MASTER_OUT,
                Operand::BufferReg(0)
            }
        };
        return {instructions, 3};
    }

    inline PatchDef create_testing_patch()
    {
        static Instruction instructions[] = {
            // 1. Audio source: WTOSC using frequency from ScalarReg(0)
            {
                OpCode::WTOSC, 
                Operand::ScalarReg(0),       // Freq from context (set by play_note)
                Operand::ScalarReg(10),      // Phase accumulator register
                Operand::Immediate_f(0.0f),  // Phase distortion
                Operand::Immediate_u(0),     // Wavetable index
                Operand::Immediate_f(0.0f),  // Morph position
                Operand::BufferReg(1)        // Audio output buffer
            },
            // 2. LFO: Triangle at 2 Hz
            {
                OpCode::LFOTRI,
                Operand::Immediate_f(2.0f),  // Frequency
                Operand::ScalarReg(5),       // Phase accumulator
                Operand::Immediate_f(0.0f),  // Phase distortion
                Operand::ShortBufReg(0)      // LFO output buffer
            },
            // 3. Scale LFO depth: LFO * 900.0
            {
                OpCode::MUL_SB,
                Operand::ShortBufReg(0),     // LFO in (-1.0 to 1.0)
                Operand::Immediate_f(900.0f),// Depth
                Operand::ShortBufReg(1)      // Scaled LFO out
            },
            // 4. Offset LFO cutoff: Scaled LFO + 1100.0
            {
                OpCode::ADD_SB,
                Operand::ShortBufReg(1),     // Scaled LFO
                Operand::Immediate_f(1100.0f),// Base cutoff frequency
                Operand::ShortBufReg(2)      // Final cutoff out
            },
            // 5. Filter: SVF LP with modulated cutoff
            {
                OpCode::SVFILTLP,
                Operand::ScalarReg(11),      // State z1
                Operand::ScalarReg(12),      // State z2
                Operand::BufferReg(1),       // Input signal
                Operand::ShortBufReg(2),     // Modulated cutoff
                Operand::Immediate_f(0.5f),  // Resonance Q
                Operand::BufferReg(1)        // Filtered audio out
            },
            // 6. ADSR: Generate envelope triggered by Gate in ScalarReg(2)
            {
                OpCode::ADSR,
                Operand::ScalarReg(2),       // Gate signal (set by play_note/release_note)
                Operand::ScalarReg(3),       // Envelope state
                Operand::ScalarReg(4),       // Envelope value
                Operand::Immediate_f(0.1f),  // Attack time (sec)
                Operand::Immediate_f(0.3f),  // Decay time (sec)
                Operand::Immediate_f(0.6f),  // Sustain level
                Operand::Immediate_f(0.5f),  // Release time (sec)
                Operand::BufferReg(2)        // Envelope output buffer
            },
            // 7. AMPL: Apply envelope to filtered audio
            {
                OpCode::AMPL,
                Operand::BufferReg(1),       // Filtered audio
                Operand::BufferReg(2),       // ADSR Envelope
                Operand::BufferReg(0)        // Output buffer
            },
            // 8. AMPL: Apply velocity scaling from ScalarReg(1)
            {
                OpCode::AMPL,
                Operand::BufferReg(0),       // Audio buffer
                Operand::ScalarReg(1),       // Velocity scalar
                Operand::BufferReg(0)        // Final audio output
            },
            // 9. MASTER_OUT: Output audio to master buffer
            {
                OpCode::MASTER_OUT,
                Operand::BufferReg(0)
            }
        };
        return {instructions, 9};
    }

    inline PatchDef acid_like_patch()
    {
        static Instruction instructions[] = {
            // 1. Audio source: WTOSC using frequency from ScalarReg(0)
            {
                OpCode::WTOSC,
                Operand::ScalarReg(0),      // Freq from context (set by play_note)
                Operand::ScalarReg(10),     // Phase accumulator register
                Operand::Immediate_f(0.0f), // Phase distortion
                Operand::Immediate_u(1),    // Wavetable index
                Operand::Immediate_f(0.0f), // Morph position
                Operand::BufferReg(1)       // Audio output buffer
            },
            // 2. LFO: Triangle at 2 Hz
            {
                OpCode::ADSR,
                Operand::ScalarReg(2),      // Gate signal (set by play_note/release_note)
                Operand::ScalarReg(3),      // Envelope state
                Operand::ScalarReg(4),      // Envelope value
                Operand::Immediate_f(0.0f), // Attack time (sec)
                Operand::Immediate_f(0.5f), // Decay time (sec)
                Operand::Immediate_f(0.0f), // Sustain level
                Operand::Immediate_f(100.f), // Release time (sec)
                Operand::ShortBufReg(0)       // Envelope output buffer
            },
            // 3. Scale LFO depth: LFO * 900.0
            {
                OpCode::MUL_SB,
                Operand::ShortBufReg(0),      // LFO in (-1.0 to 1.0)
                Operand::Immediate_f(8000.0f), // Depth
                Operand::ShortBufReg(1)       // Scaled LFO out
            },
            // 4. Offset LFO cutoff: Scaled LFO + 1100.0
            {
                OpCode::ADD_SB,
                Operand::ShortBufReg(1),       // Scaled LFO
                Operand::Immediate_f(1000.0f), // Base cutoff frequency
                Operand::ShortBufReg(2)        // Final cutoff out
            },
            // 5. Filter: SVF LP with modulated cutoff
            {
                OpCode::SVFILTLP,
                Operand::ScalarReg(11),     // State z1
                Operand::ScalarReg(12),     // State z2
                Operand::BufferReg(1),      // Input signal
                Operand::ShortBufReg(2),    // Modulated cutoff
                Operand::Immediate_f(0.9f), // Resonance Q
                Operand::BufferReg(1)       // Filtered audio out
            },
            // 6. MASTER_OUT: Output audio to master buffer
            {
                OpCode::MASTER_OUT,
                Operand::BufferReg(1)
            }
        };
        return {instructions, 6};
    }

    inline PatchDef two_table_patch()
    {
        static Instruction instructions[] = {
            // 1. Audio source: WTOSC using frequency from ScalarReg(0)
            {
                OpCode::WTOSC,
                Operand::ScalarReg(0),      // Freq from context (set by play_note)
                Operand::ScalarReg(10),     // Phase accumulator register
                Operand::Immediate_f(0.0f), // Phase distortion
                Operand::Immediate_u(4),    // Wavetable index
                Operand::Immediate_f(0.0f), // Morph position
                Operand::BufferReg(1)       // Audio output buffer
            },
            // 2. Second wavetable
            {
                OpCode::WTOSC,
                Operand::ScalarReg(0),      // Freq from context (set by play_note)
                Operand::ScalarReg(11),     // Phase accumulator register
                Operand::Immediate_f(0.0f), // Phase distortion
                Operand::Immediate_u(5),    // Wavetable index
                Operand::Immediate_f(0.0f), // Morph position
                Operand::BufferReg(2)       // Audio output buffer
            },
            // 3. adsr 1
            {
                OpCode::ADSR,
                Operand::ScalarReg(2),      // Gate signal (set by play_note/release_note)
                Operand::ScalarReg(3),      // Envelope state
                Operand::ScalarReg(4),      // Envelope value
                Operand::Immediate_f(0.0f), // Attack time (sec)
                Operand::Immediate_f(1.0f), // Decay time (sec)
                Operand::Immediate_f(0.6f), // Sustain level
                Operand::Immediate_f(0.5f), // Release time (sec)
                Operand::ShortBufReg(1)       // Envelope output buffer
            },
            // 4. adsr 2
            {
                OpCode::ADSR,
                Operand::ScalarReg(2),      // Gate signal (set by play_note/release_note)
                Operand::ScalarReg(5),      // Envelope state
                Operand::ScalarReg(6),      // Envelope value
                Operand::Immediate_f(0.0f), // Attack time (sec)
                Operand::Immediate_f(0.5f), // Decay time (sec)
                Operand::Immediate_f(0.0f), // Sustain level
                Operand::Immediate_f(0.5f), // Release time (sec)
                Operand::ShortBufReg(2)       // Envelope output buffer
            },
            // 5. Mix 1 and 2
            {
                OpCode::MIX,
                Operand::BufferReg(1),      // Input buffer 1
                Operand::BufferReg(2),      // Input buffer 2
                Operand::ShortBufReg(1),    // Amount from 1
                Operand::ShortBufReg(2),    // Amount from 2
                Operand::BufferReg(3)       // Output buffer
            },
            // 6. double the note frequency and store it in register 15
            {
                OpCode::MUL,
                Operand::ScalarReg(0),
                Operand::Immediate_f(2.0),
                Operand::ScalarReg(15)
            },
            // 7. filter that uses the doubled frequency for cutoff
            {
                OpCode::SVFILTLP,
                Operand::ScalarReg(13),     // State z1
                Operand::ScalarReg(14),     // State z2
                Operand::BufferReg(3),      // Input signal
                Operand::ScalarReg(15),    // Modulated cutoff
                Operand::Immediate_f(0.8f), // Resonance Q
                Operand::BufferReg(0)       // Filtered audio out
            },
            { // 8. soft clip distortion
                OpCode::SOFTCLIP,
                Operand::BufferReg(0),
                Operand::Immediate_f(1.0f),
                Operand::BufferReg(0)
            },
            // 9. MASTER_OUT: Output audio to master buffer
            {
                OpCode::MASTER_OUT,
                Operand::BufferReg(0)
            }};
        return {instructions, 9};
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
