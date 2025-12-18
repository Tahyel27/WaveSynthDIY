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
#include <Controller/Controller.hpp>
#include "ADSR.hpp"
#include "HWProfiler.hpp"

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

    HWProfiler::init();

    auto irqHandler = IRQHandler::getIRQHandler();

    //pin H has the lowest number button
    auto device = AudioDevice(12,13,AudioDevice::DeviceMode::STEREO,1,irqHandler);

    //data 19, clk 20, latch 21
    auto btnarr = ButtonArray(19, 20, 21);

    auto engine = Synth::SynthEngine();
    /*auto [data, ord] = engine.getDataForVoiceRef(0);
    Synth::createPatchAlgo1(data, ord, 70, 1200, 0.15);
    engine.startVoice(0);
    engine.setDelay(false);*/
    engine.setDelay(true);

    auto analog = AnalogArray(16, 17, 18, 26);

    auto seq = Sequencer(&engine);

    //InstrumentManager manager(&engine);
    auto manager = seq.getManager();

    Synth::Data data; Synth::NodeOrder ord;
    //Synth::createPatchAlgo1(data, ord, 70, 1200, 0.15);
    Synth::createPatchAlgo2(data, ord);
    //Synth::createSimpleWTPatch(data, ord, 0);
    //Synth::createWTPatchwithFilter(data, ord);

    manager->setInstrument(data, ord, 0);
    //manager.playFrequency(100, 0, 1);

    //device.setSource(&sine);

    device.setSource(&engine);

    printf("start device init\n");

    device.initialize();

    int timer = 0;

    manager->updateInstrument([](Synth::Data &d){
        d.SVFArr[0].fenv = 2000;
        d.SVFArr[0].Q = 0.2;
        d.WTOscArr[0].wtIndex = 1;
        d.WTOscArr[0].unison = 3;
        d.WTOscArr[0].detune.v = 0.6;
        d.WTOscArr[0].phaseDistMod.v = 0;
        d.ADSRArr[0].sustain = 0;
        d.ADSRArr[0].decay = 0.2;
        d.ADSRArr[1].attack = 0.;
        d.ADSRArr[1].sustain = 0;
        d.ADSRArr[1].decay = 0.7;
    },0);

    //manager.playFrequency(440, 0, 1);
    //manager.playFrequency(200, 0, 2);
    //manager.playFrequency(110, 0, 3);

    std::array<float, 8> cMajorScale = {
        130.81f, // C3
        146.83f, // D3
        164.81f, // E3
        174.61f, // F3
        196.00f, // G3
        220.00f, // A3
        246.94f, // B3
        261.63f  // C4
    };


    while (true)
    {
        if(device.update())
        {
            seq.tick();
            /*float v = analog.readChannelVoltageStable(4);

            manager.updateInstrument([&v](Synth::Data &d){
                //d.WTOscArr[0].freq.v = 200*v + 100;
                //d.SineOscArr[0].freq.v = 200*v + 100;
                d.SVFArr[0].fenv = v*200;
            },0);

            v = analog.readChannelVoltage(0);
            manager.updateInstrument([&v](Synth::Data &d){
                //d.SVFArr[0].fcut = 400 + 200*v;
                d.SVFArr[0].Q = (3.6 - v)/3.6;
                //d.WTOscArr[0].phaseDistMod.v = (3.4 - v) / 3.4;
                //d.SineOscArr[1].freq.v = v*3;
            },0);*/

            //printf("fcut: %f\n", (3.4 - v) / 3.4);
            btnarr.poll();

            auto b = btnarr.getEvent();
            while (b.has_value())
            {
                if (b.value().type == ButtonEvent::Type::PRESSED)
                {
                    manager->playFrequency(2*cMajorScale[b.value().button], 0, b.value().button);
                }

                if (b.value().type == ButtonEvent::Type::RELEASED)
                {
                    manager->release(b.value().button);
                }
                
                b = btnarr.getEvent();
            }

            /*timer++;

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
            }*/
        }
        
    }
}
