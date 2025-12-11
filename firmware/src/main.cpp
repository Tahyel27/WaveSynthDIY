#include <stdio.h>
#include <cmath>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "AudioDevice.hpp"
#include "AnalogArray.hpp"
#include "ButtonArray.hpp"
#include "WavetableSynth.hpp"
#include <SynthCore/Engine.hpp>
#include <SynthCore/Patches.hpp>
#include <InstrumentManager.hpp>
#include "ADSR.hpp"

struct sineSynth
{
    uint64_t samplecounter;

    int16_t operator()(double f, int amp, uint64_t SPS)
    {
        double t = double(samplecounter)/double(SPS);
        samplecounter += 1*f;
        if (samplecounter > SPS)
        {
            samplecounter = samplecounter - SPS;
        }
        
        return amp*sin(2*t*M_PI);
    }
};

struct sineSource : public AudioSource
{
    sineSynth synth;
    
    void audioCallback(AudioBuffer buffer) override
    {
        for (size_t i = 0; i < buffer.buffsize; i++)
        {
            buffer.write16bit(i, synth(440, buffer.maxamp / 4, buffer.SPS), AudioBuffer::Mode::MONO);
        }
    }
};

int main()
{
    stdio_init_all();

    auto irqHandler = IRQHandler::getIRQHandler();

    //pin H has the lowest number button
    auto device = AudioDevice(12,13,AudioDevice::DeviceMode::STEREO,1,irqHandler);

    auto engine = Synth::SynthEngine();
    /*auto [data, ord] = engine.getDataForVoiceRef(0);
    Synth::createPatchAlgo1(data, ord, 70, 1200, 0.15);
    engine.startVoice(0);
    engine.setDelay(false);*/

    InstrumentManager manager(&engine);

    Synth::Data data; Synth::NodeOrder ord;
    Synth::createPatchAlgo1(data, ord, 70, 1200, 0.15);
    manager.setInstrument(data, ord, 0);
    manager.playFrequency(100, 0, 1);

    device.setSource(&engine);

    device.initialize();

    int timer = 0;

    while (true)
    {
        if(device.update())
        {
            timer++;

            if (timer == 100)
            {
                manager.release(1);
            }

            if (timer == 200)
            {
                manager.playFrequency(200, 0, 1);
            }

            if (timer == 250)
            {
                manager.playFrequency(400, 0, 2);
            }

            if (timer == 300)
            {
                manager.release(1);
                manager.release(2);
            }

            if (timer == 400)
            {
                manager.playFrequency(100, 0, 1);
                timer = 0;
            }
        }

    }
}
