#include <iostream>
#include <portaudio.h>
#include "SynthCore/Engine.hpp"
#include "SynthCore/Patches.hpp"

using namespace Synth;

// PortAudio callback
static int paCallback(const void *inputBuffer, void *outputBuffer,
                      unsigned long framesPerBuffer,
                      const PaStreamCallbackTimeInfo* timeInfo,
                      PaStreamCallbackFlags statusFlags,
                      void *userData)
{
    SynthEngine* engine = (SynthEngine*)userData;
    uint32_t* out = (uint32_t*)outputBuffer;

    AudioBuffer ab;
    ab.buffer = out;
    ab.buffsize = framesPerBuffer;
    ab.SPS = Synth::SPS; 
    ab.maxamp = Synth::maxamp; 

    // The engine's audioCallback expects to process CHUNKS_PER_BUFFER chunks,
    // which results in processing exactly BUFFER_SIZE samples.
    if (framesPerBuffer == Synth::BUFFER_SIZE) {
        engine->audioCallback(ab);
    } else {
        // If portaudio doesn't give us exactly the buffer size, 
        // we just output silence (or we could handle partial buffers, 
        // but for testing fixed sizes are usually fine).
        for (unsigned int i = 0; i < framesPerBuffer * 2; ++i) {
            out[i] = 0;
        }
    }

    return paContinue;
}


int main() {
    std::cout << "Initializing Synth Engine...\n";

    BufferPool pool;
    ShortBufferPool short_pool;
    ScalarRegister ext_reg;
    SynthEngine engine(&pool, &short_pool, &ext_reg);


    Instruction instructions[] = {
        // 1. Audio source: WTOSC 440 Hz
        {
            OpCode::WTOSC, 
            Operand::Immediate_f(440.), 
            Operand::ScalarReg(0), 
            Operand::Immediate_f(0.), 
            Operand::Immediate_u(1), 
            Operand::Immediate_f(0.), 
            Operand::BufferReg(1)
        },
        // 2. LFO: Triangle at 2 Hz
        {
            OpCode::LFOTRI,
            Operand::Immediate_f(2.0f),  // freq
            Operand::ScalarReg(5),       // phi
            Operand::Immediate_f(0.0f),  // phasedist
            Operand::ShortBufReg(0)      // out
        },
        // 3. Scale LFO depth: LFO * 900.0
        {
            OpCode::MUL_SB,
            Operand::ShortBufReg(0),     // LFO in (-1.0 to 1.0)
            Operand::Immediate_f(900.0f),// Depth (yields -900.0 to 900.0)
            Operand::ShortBufReg(1)      // Scaled LFO out
        },
        // 4. Offset LFO: Scaled LFO + 1100.0
        {
            OpCode::ADD_SB,
            Operand::ShortBufReg(1),     // Scaled LFO
            Operand::Immediate_f(1100.0f),// Base frequency (yields 200.0 to 2000.0)
            Operand::ShortBufReg(2)      // Final cutoff out
        },
        // 5. Filter: SVF LP with modulated cutoff
        {
            OpCode::SVFILTLP,
            Operand::ScalarReg(1),       // z1
            Operand::ScalarReg(2),       // z2
            Operand::BufferReg(1),       // input signal
            Operand::ShortBufReg(2),     // modulated cutoff
            Operand::Immediate_f(0.5f),  // q (resonance)
            Operand::BufferReg(1)        // output signal (overwrite)
        },
        // 6. ADSR: Generate envelope
        {
            OpCode::ADSR,
            Operand::Immediate_f(1.0f),  // Gate is HIGH
            Operand::ScalarReg(3),       // State
            Operand::ScalarReg(4),       // Env Val
            Operand::Immediate_f(0.5f),  // Attack
            Operand::Immediate_f(0.5f),  // Decay
            Operand::Immediate_f(0.3f),  // Sustain
            Operand::Immediate_f(1.0f),  // Release
            Operand::BufferReg(2)        // Env Out
        },
        // 7. AMPL: Apply envelope to filtered audio
        {
            OpCode::AMPL,
            Operand::BufferReg(1),       // Filtered audio
            Operand::BufferReg(2),       // Envelope
            Operand::BufferReg(0)        // Final Output
        }
    };

    engine.set_instructions(instructions, 7);

    std::cout << "Initializing PortAudio...\n";
    PaError err = Pa_Initialize();
    if (err != paNoError) {
        std::cerr << "PortAudio error: " << Pa_GetErrorText(err) << "\n";
        return 1;
    }

    PaStream *stream;
    err = Pa_OpenDefaultStream(&stream,
                               0,          /* no input channels */
                               2,          /* stereo output */
                               paInt32,    /* 32 bit int output */
                               Synth::SPS, /* sample rate */
                               Synth::BUFFER_SIZE, /* frames per buffer */
                               paCallback,
                               &engine);
                               
    if (err != paNoError) {
        std::cerr << "PortAudio error: " << Pa_GetErrorText(err) << "\n";
        Pa_Terminate();
        return 1;
    }

    err = Pa_StartStream(stream);
    if (err != paNoError) {
        std::cerr << "PortAudio error: " << Pa_GetErrorText(err) << "\n";
        Pa_CloseStream(stream);
        Pa_Terminate();
        return 1;
    }

    std::cout << "Playing audio. Press Enter to stop.\n";
    std::cin.get();

    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();

    return 0;
}
