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
    
    // Set up a custom continuous patch to fix firmware bugs (unison=0, unhandled ADSR SUSTAIN)
    Data data;
    NodeOrder order;


    Instruction instructions[] = {
        {
            OpCode::WTOSC, 
            Operand::ExternalReg(0), 
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
            Operand::Immediate_f(1000.),
            Operand::Immediate_f(1.),
            Operand::BufferReg(1) // Overwrite buffer 1 with filtered signal
        },
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
        {
            OpCode::AMPL,
            Operand::BufferReg(1),       // Filtered audio
            Operand::BufferReg(2),       // Envelope
            Operand::BufferReg(0)        // Final Output
        }
    };
    
    ext_reg[0].f = 800.; //WTOSC frequency

    order.nodeCount = 1;
    
    WTOscData osc;
    osc.wtIndex = 1;
    osc.unison = 1;
    osc.detune = ModInput{-1, 0.3f};
    osc.phaseDistMod = ModInput{-1, 0.0f};
    osc.freq = ModInput{-1, 200.0f};
    osc.morph = ModInput{-1, 0.0f};

    data.WTOscArr[0] = osc;
    order.data[0].type = NodeType::WTOSCILLATOR;
    order.data[0].dataIndex = 0;
    order.data[0].outputBuffer = -1; // -1 routes directly to the voice output

    engine.loadData(data);
    engine.loadOrdering(order);
    engine.set_instructions(instructions, 4);

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
