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
    SynthEngine engine;
    
    // Set up a custom continuous patch to fix firmware bugs (unison=0, unhandled ADSR SUSTAIN)
    Data data;
    NodeOrder order;
    
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

    engine.loadVoiceData(data, order, 0);
    engine.startVoice(0);

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
