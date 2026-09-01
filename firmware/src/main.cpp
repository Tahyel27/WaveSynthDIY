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
#include "ADSR.hpp"
#include "HWProfiler.hpp"
#include "Events.hpp"
#include "StaticQueue.hpp"
#include "AudioStack.hpp"

int main()
{
    stdio_init_all();

    HWProfiler::init();

    auto buffers = AudioDeviceBuffers();

    auto irqHandler = IRQHandler::getIRQHandler();

    //pin H has the lowest number button
    auto device_opt = AudioDevice::claim(12, 13, &buffers, irqHandler);
    if (!device_opt.has_value()) return -1;

    auto device = std::move(device_opt.value());

    auto buffer_pool = BufferPool();
    auto short_buffer_pool = ShortBufferPool();
    auto ext_register = ScalarRegister();

    auto poly_manager = PolyphonyManager(&buffer_pool, &short_buffer_pool, &ext_register);
    auto fx_stack = EffectStack(EffectStackConfig{.hard_clip = true, .hard_clip_gain = 1.0f});
    auto audio_stack = AudioStack(poly_manager, fx_stack);

    auto patch = Synth::create_testing_patch();
    poly_manager.set_instructions(patch.instructions, 9);

    device.setSource(&audio_stack);


    //data 19, clk 20, latch 21
    //auto btnarr = ButtonArray(19, 20, 21);

    auto btnarr_opt = ButtonArray::claim(19,20,21);
    if (!btnarr_opt.has_value())
    {
        return -1;
    }

    auto btnarr = std::move(btnarr_opt.value());
    

    /*auto [data, ord] = engine.getDataForVoiceRef(0);
    Synth::createPatchAlgo1(data, ord, 70, 1200, 0.15);
    engine.startVoice(0);
    engine.setDelay(false);*/

    auto analog = AnalogArray(16, 17, 18, 26);


    //manager.playFrequency(100, 0, 1);

    //device.setSource(&sine);



    printf("start device init\n");


    int timer = 0;

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

    auto queue = staticQueue<Event, 20>();

    auto encoderArr = EncoderArray(5, 4, 3);



    int noteA = 0;
    int noteB = 0;

    while (true)
    {
        if(device.update())
        {
            //seq.tick();
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
            /*btnarr.poll();

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
            HWProfiler::putHI();

            encoderArr.pollEvents(queue);
            if (!queue.empty())
            {
                auto ev = queue.pop();
                if (ev.type == Event::Type::ENCODER_LEFT)
                {
                    printf("LEFT Encoder value: %d\n", ev.ID);
                }
                else
                {
                    printf("RIGHT Encoder value: %d\n", ev.ID);
                }
            }*/

            btnarr.poll();
            auto b = btnarr.getEvent();
            while (b.has_value())
            {
                printf("button %d ", b.value().button);
                if (b.value().type == ButtonEvent::Type::PRESSED) 
                    printf("pressed\n"); else printf("released\n");

                b = btnarr.getEvent();
            }

            HWProfiler::putLO();
            timer++;

            if (timer == 200)
            {
                noteA = poly_manager.play_note(200.0f);
            }

            if (timer == 250)
            {
                noteB = poly_manager.play_note(400.0f);
            }

            if (timer == 300)
            {
                poly_manager.release_note(noteA);
                poly_manager.release_note(noteB);
            }

            if (timer == 400)
            {
                noteA = poly_manager.play_note(100.0f);
            }

            if (timer == 500)
            {
                poly_manager.release_note(noteA);
                timer = 0;
            }
        }
        
    }
}
