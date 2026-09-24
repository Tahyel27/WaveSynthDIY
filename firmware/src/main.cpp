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
#include "Encoder.hpp"
#include "StaticQueue.hpp"
#include "AudioStack.hpp"
#include "MIDIUart.hpp"
#include "SPIHandler.hpp"
#include "ButtonMatrix.hpp"

static const float MIDI_NOTES[128] = {
    8.1758f, 8.6620f, 9.1770f, 9.7227f, 10.3009f, 10.9134f, 11.5623f, 12.2499f,
    12.9783f, 13.7500f, 14.5676f, 15.4339f, 16.3516f, 17.3239f, 18.3540f, 19.4454f,
    20.6017f, 21.8268f, 23.1247f, 24.4997f, 25.9565f, 27.5000f, 29.1352f, 30.8677f,
    32.7032f, 34.6478f, 36.7081f, 38.8909f, 41.2034f, 43.6535f, 46.2493f, 48.9994f,
    51.9131f, 55.0000f, 58.2705f, 61.7354f, 65.4064f, 69.2957f, 73.4162f, 77.7817f,
    82.4069f, 87.3071f, 92.4986f, 97.9989f, 103.8262f, 110.0000f, 116.5409f, 123.4708f,
    130.8128f, 138.5913f, 146.8324f, 155.5635f, 164.8138f, 174.6141f, 184.9972f, 195.9977f,
    207.6523f, 220.0000f, 233.0819f, 246.9417f, 261.6256f, 277.1826f, 293.6648f, 311.1270f,
    329.6276f, 349.2282f, 369.9944f, 391.9954f, 415.3047f, 440.0000f, 466.1638f, 493.8833f,
    523.2511f, 554.3653f, 587.3295f, 622.2540f, 659.2551f, 698.4565f, 739.9888f, 783.9909f,
    830.6094f, 880.0000f, 932.3275f, 987.7666f, 1046.5023f, 1108.7305f, 1174.6591f, 1244.5079f,
    1318.5103f, 1396.9129f, 1479.9777f, 1567.9817f, 1661.2188f, 1760.0000f, 1864.6550f, 1975.5332f,
    2093.0045f, 2217.4610f, 2349.3181f, 2489.0159f, 2637.0205f, 2793.8259f, 2959.9554f, 3135.9635f,
    3322.4376f, 3520.0000f, 3729.3101f, 3951.0664f, 4186.0090f, 4434.9221f, 4698.6363f, 4978.0317f,
    5274.0410f, 5587.6517f, 5919.9108f, 6271.9270f, 6644.8752f, 7040.0000f, 7458.6202f, 7902.1328f,
    8372.0181f, 8869.8442f, 9397.2726f, 9956.0635f, 10548.082f, 11175.303f, 11839.822f, 12543.854f};


const auto ENCODER_PRIMARY_PIO = pio0;

const uint ENCODER_1_AB = 16;
const uint ENCODER_2_AB = 26; 

const uint PCM5102A_DATA = 12;
const uint PCM5102A_LCK = 13;

const auto BUTTON_SPI = spi0;
const uint BUTTON_IN_SPI_MISO = 0;
const uint BUTTON_IN_SPI_MOSI = 3;
const uint BUTTON_IN_SPI_SCK = 2;

const uint BUTTON_OUT_DATA = 19;
const uint BUTTON_OUT_CLOCK = 20;
const uint BUTTON_OUT_LATCH = 21;

const uint MIDI_RX = 5;

int main()
{
    stdio_init_all();

    HWProfiler::init();

    sleep_ms(2000);

    //encoders need to be the first pio acquired
    //pins 16 and 17
    auto encoder_opt = Encoder::acquire_first(ENCODER_PRIMARY_PIO, ENCODER_1_AB, 0);
    if (!encoder_opt.has_value()) return -1;
    auto encoder = std::move(encoder_opt.value());

    auto encoder2_opt = Encoder::acquire_other(encoder, ENCODER_2_AB, 1, 1);
    if (!encoder2_opt.has_value()) return -1;
    auto encoder2 = std::move(encoder2_opt.value());

    auto buffers = AudioDeviceBuffers();

    auto irqHandler = IRQHandler::getIRQHandler();

    //pin H has the lowest number button
    auto device_opt = AudioDevice::claim(PCM5102A_DATA, PCM5102A_LCK, &buffers, irqHandler);
    if (!device_opt.has_value()) return -1;

    auto device = std::move(device_opt.value());

    auto buffer_pool = BufferPool();
    auto short_buffer_pool = ShortBufferPool();
    auto ext_register = ScalarRegister();

    auto poly_manager = PolyphonyManager(&buffer_pool, &short_buffer_pool, &ext_register);
    auto fx_stack = EffectStack(EffectStackConfig{.hard_clip = true, .hard_clip_gain = 1.0f});
    auto audio_stack = AudioStack(poly_manager, fx_stack);

    auto patch = Synth::two_table_patch();
    poly_manager.set_instructions(patch.instructions, patch.count);

    device.setSource(&audio_stack);


    //data 19, clk 20, latch 21
    //auto btnarr = ButtonArray(19, 20, 21);

    /*auto btnarr_opt = ButtonArray::claim(19,20,21);
    if (!btnarr_opt.has_value())
    {
        return -1;
    }
    auto btnarr = std::move(btnarr_opt.value());
    */
    auto spi = SPIHandler(
        BUTTON_SPI, 
        BUTTON_IN_SPI_MISO,
        BUTTON_IN_SPI_MOSI,
        BUTTON_IN_SPI_SCK,
        4 * 1000 * 1000
    );
    auto matrix_opt = ButtonMatrix::claim(
        &spi, 
        BUTTON_OUT_DATA, 
        BUTTON_OUT_CLOCK, 
        BUTTON_OUT_LATCH, 
        4, 
        8
    );
    if (!matrix_opt.has_value())
    {
        return -1;
    }
    auto matrix = std::move(matrix_opt.value());

    auto midi_receiver = MidiReceiver::acquire_uart1(MIDI_RX);
    

    /*auto [data, ord] = engine.getDataForVoiceRef(0);
    Synth::createPatchAlgo1(data, ord, 70, 1200, 0.15);
    engine.startVoice(0);
    engine.setDelay(false);*/

    //auto analog = AnalogArray(16, 17, 18, 26);


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

    /*auto encoderArr_opt = EncoderArray::claim(5, 4, 3);
    if(!encoderArr_opt.has_value()) return -1;

    auto encoderArr = std::move(encoderArr_opt.value());*/

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

            //btnarr.poll(queue);
            matrix.poll(queue);
            encoder.poll(queue);
            encoder2.poll(queue);
            midi_receiver.poll(queue);
            while (!queue.empty())
            {
                auto ev = queue.pop();
                
                if (ev.is_type(EventType::BUTTON_PRESS))
                    printf("button %d pressed\n", ev.get_button_press());
                if (ev.is_type(EventType::BUTTON_RELEASE))
                    printf("button %d released\n", ev.get_button_release());
                if (ev.is_type(EventType::ENCODER_TURN))
                    printf("encoder %d turned: %d\n", ev.get_encoder_turn().encoder_id, ev.get_encoder_turn().change);
                if (ev.is_type(EventType::MIDI_MESSAGE))
                {
                    auto msg = ev.get_midi_message();
                    if (msg.data_2 == 0)
                        audio_stack.send_command(ReleaseNote(msg.data_1));
                    else
                        audio_stack.send_command(PressNote(msg.data_1, MIDI_NOTES[msg.data_1]));
                    printf("Midi message received: %x %x %x\n",msg.status, msg.data_1, msg.data_2);
                }
            }

            HWProfiler::putLO();
            timer++;

            /*if (timer == 200)
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
            }*/
        }
        
    }
}
