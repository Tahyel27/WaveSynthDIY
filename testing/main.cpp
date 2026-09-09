#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <portaudio.h>
#include "SynthCore/Engine.hpp"
#include "SynthCore/Patches.hpp"
#include "PolyphonyManager.hpp"
#include "EffectStack.hpp"
#include "AudioStack.hpp"
#include "AudioCommands.hpp"

using namespace Synth;

// PortAudio callback using PolyphonyManager
static int paCallback(const void *inputBuffer, void *outputBuffer,
                      unsigned long framesPerBuffer,
                      const PaStreamCallbackTimeInfo* timeInfo,
                      PaStreamCallbackFlags statusFlags,
                      void *userData)
{
    AudioStack* stack = static_cast<AudioStack*>(userData);
    uint32_t* out = static_cast<uint32_t*>(outputBuffer);

    if (framesPerBuffer == Synth::BUFFER_SIZE) {

        AudioBuffer ab;
        ab.buffer = out;
        ab.buffsize = framesPerBuffer;
        ab.SPS = Synth::SPS; 
        ab.maxamp = Synth::maxamp; 

        stack->audioCallback(ab);
    } else {
        for (unsigned int i = 0; i < framesPerBuffer * 2; ++i) {
            out[i] = 0;
        }
    }

    return paContinue;
}

int main() {
    std::cout << "Initializing PolyphonyManager...\n";

    BufferPool pool;
    ShortBufferPool short_pool;
    ScalarRegister ext_reg;

    PolyphonyManager manager{&pool, &short_pool, &ext_reg};
    EffectStack fx_stack{EffectStackConfig{.hard_clip = true, .hard_clip_gain = 1.0f} };
    auto audio_stack = AudioStack(manager, fx_stack);

    // Instruction patch designed for PolyphonyManager:
    // Frequency is passed in ScalarReg(0) via play_note()
    // Velocity is passed in ScalarReg(1) via play_note()
    // Gate signal is passed in ScalarReg(2) via set_gate() / play_note()
    Instruction instructions[] = {
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

    manager.set_instructions(instructions, 9);
    manager.set_release_samples(static_cast<uint32_t>(0.5f * Synth::SPS));

    std::cout << "Initializing PortAudio...\n";
    PaError err = Pa_Initialize();
    if (err != paNoError) {
        std::cerr << "PortAudio error: " << Pa_GetErrorText(err) << "\n";
        return 1;
    }

    PaStream *stream;
    err = Pa_OpenDefaultStream(&stream,
                               0,                  /* no input channels */
                               2,                  /* stereo output */
                               paInt32,            /* 32 bit int output */
                               Synth::SPS,         /* sample rate */
                               Synth::BUFFER_SIZE, /* frames per buffer */
                               paCallback,
                               &audio_stack);
                               
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

    std::cout << "\n=== PolyphonyManager Test Started ===\n";
    
    // Test 1: Arpeggio (playing notes sequentially to build a chord)
    std::cout << "1. Playing C Major chord arpeggio (C4, E4, G4, C5)...\n";
    int v1 = manager.play_note(261.63f, 0.8f); // C4
    std::this_thread::sleep_for(std::chrono::milliseconds(400));
    
    int v2 = manager.play_note(329.63f, 0.8f); // E4
    std::this_thread::sleep_for(std::chrono::milliseconds(400));
    
    int v3 = manager.play_note(392.00f, 0.8f); // G4
    std::this_thread::sleep_for(std::chrono::milliseconds(400));
    
    int v4 = manager.play_note(523.25f, 1.0f); // C5
    std::cout << "   Polyphonic chord active with 4 voices!\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(1200));

    // Test 2: Releasing notes
    std::cout << "2. Releasing all 4 notes...\n";
    manager.release_note(v1);
    manager.release_note(v2);
    manager.release_note(v3);
    manager.release_note(v4);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    // Test 3: Two notes being played through the stack
    std::cout << "testing two note through stack\n";
    audio_stack.send_command(PressNote{.play_ID = 0, .frequency = 440.f, .velocity = 1.0});
    std::this_thread::sleep_for(std::chrono::milliseconds(400));
    audio_stack.send_command(PressNote{.play_ID = 1, .frequency = 329.63f, .velocity = 1.0});
    std::this_thread::sleep_for(std::chrono::milliseconds(400));
    audio_stack.send_command(ReleaseNote{.play_ID = 1});
    std::this_thread::sleep_for(std::chrono::milliseconds(400));
    audio_stack.send_command(ReleaseNote{.play_ID = 0});
    std::this_thread::sleep_for(std::chrono::milliseconds(400));

    // Test 4: Playing a 6-voice full chord (utilizing maximum polyphony VOICE_COUNT = 6)
    std::cout << "3. Playing 6-voice full chord (C3, G3, C4, E4, G4, B4)...\n";
    std::vector<int> chord_voices;
    chord_voices.push_back(manager.play_note(130.81f, 0.9f)); // C3
    chord_voices.push_back(manager.play_note(196.00f, 0.8f)); // G3
    chord_voices.push_back(manager.play_note(261.63f, 0.7f)); // C4
    chord_voices.push_back(manager.play_note(329.63f, 0.7f)); // E4
    chord_voices.push_back(manager.play_note(392.00f, 0.7f)); // G4
    chord_voices.push_back(manager.play_note(493.88f, 0.8f)); // B4

    std::cout << "Playing 6-voice chord. Press Enter to stop test.\n";
    std::cin.get();

    // Clean up notes
    for (int voice_id : chord_voices) {
        if (voice_id >= 0) {
            manager.release_note(voice_id);
        }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(600));

    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();

    std::cout << "PolyphonyManager test completed.\n";
    return 0;
}
