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
#include "UI/UI.hpp"


const auto ENCODER_PRIMARY_PIO = pio0;
const auto ENCODER_SECONDARY_PIO = pio1;

const uint ENCODER_1_AB = 16;
const uint ENCODER_2_AB = 26; 
const uint ENCODER_3_AB = 6;
const uint ENCODER_4_AB = 9;
const uint ENCODER_5_AB = 0;

const uint PCM5102A_DATA = 12;
const uint PCM5102A_LCK = 13;
//         PCM5102A_BCK = 14

const auto BUTTON_SPI = spi0;
const uint BUTTON_IN_SPI_MISO = 0;
const uint BUTTON_IN_SPI_MOSI = 3;
const uint BUTTON_IN_SPI_SCK = 2;
const uint BUTTON_IN_SPI_CS = 4;

const uint BUTTON_OUT_DATA = 19;
const uint BUTTON_OUT_CLOCK = 20;
const uint BUTTON_OUT_LATCH = 21;

const uint MIDI_RX = 5;

int main()
{
    stdio_init_all();

    HWProfiler::init();

    sleep_ms(500);

    //encoders need to be the first pio acquired
    //pins 16 and 17
    auto encoder_opt = Encoder::acquire_first(ENCODER_PRIMARY_PIO, ENCODER_1_AB, 0);
    if (!encoder_opt.has_value()) return -1;
    auto encoder = std::move(encoder_opt.value());

    auto encoder2_opt = Encoder::acquire_other(encoder, ENCODER_2_AB, 1, 1);
    if (!encoder2_opt.has_value()) return -1;
    auto encoder2 = std::move(encoder2_opt.value());

    auto encoder3_opt = Encoder::acquire_other(encoder, ENCODER_3_AB, 2, 2);
    if (!encoder3_opt.has_value()) return -1;
    auto encoder3 = std::move(encoder3_opt.value());

    auto encoder4_opt = Encoder::acquire_other(encoder, ENCODER_4_AB, 3, 3);
    if (!encoder4_opt.has_value()) return -1;
    auto encoder4 = std::move(encoder4_opt.value());

    auto encoder5_opt = Encoder::acquire_first(ENCODER_SECONDARY_PIO, ENCODER_5_AB, 4);
    if (!encoder5_opt.has_value()) return -1;
    auto encoder5 = std::move(encoder5_opt.value());

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
    poly_manager.set_release_samples(22000);
    auto fx_stack = EffectStack(EffectStackConfig{.hard_clip = true, .soft_clip = false, .hard_clip_gain = 1.0f});
    auto audio_stack = AudioStack(poly_manager, fx_stack);

    auto patch = Synth::two_table_patch();
    poly_manager.set_instructions(patch.instructions, patch.count);

    device.setSource(&audio_stack);

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
        BUTTON_IN_SPI_CS, 
        4, 
        8
    );
    if (!matrix_opt.has_value())
    {
        return -1;
    }
    auto matrix = std::move(matrix_opt.value());

    auto midi_receiver = MidiReceiver::acquire_uart1(MIDI_RX);

    auto ui = UI();

    ui.init_ui(audio_stack);

    printf("start device init\n");

    auto queue = staticQueue<Event, 20>();

    int timer = 0;

    constexpr int NUM_VOICES = 6;      // Set your 'n' here
    constexpr int START_TICK = 200;    // Tick to begin playing notes
    constexpr int NOTE_STAGGER = 20;   // Ticks between consecutive note presses
    constexpr int HOLD_DURATION = 100; // How long all notes hold after the last one is triggered
    constexpr int RELEASE_GAP = 120;   // Silence duration before repeating the test

    // Calculated milestone ticks
    constexpr int ALL_PRESSED_TICK = START_TICK + (NUM_VOICES - 1) * NOTE_STAGGER;
    constexpr int RELEASE_TICK = ALL_PRESSED_TICK + HOLD_DURATION;
    constexpr int RESET_TICK = RELEASE_TICK + RELEASE_GAP;

    while (true)
    {
        if(device.update())
        {
            matrix.poll(queue);
            encoder.poll(queue);
            encoder2.poll(queue);
            encoder3.poll(queue);
            encoder4.poll(queue);
            encoder5.poll(queue);
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
                    printf("Midi message received: %x %x %x\n",msg.status, msg.data_1, msg.data_2);
                }

                ui.process_event(ev, audio_stack);
            }

            /*timer++;

            if (timer >= START_TICK && timer <= ALL_PRESSED_TICK)
            {
                int offset = timer - START_TICK;
                if (offset % NOTE_STAGGER == 0)
                {
                    int voice_idx = (offset / NOTE_STAGGER) + 1; // 1-indexed note ID
                    float freq = 220.0f + (voice_idx * 110.0f);  // Or pick frequencies from an array / formula

                    audio_stack.send_command(PressNote(voice_idx, freq));
                    printf("playing voice %d \n", voice_idx);
                }
            }

            // 2. All Notes Off: release every active voice simultaneously
            if (timer == RELEASE_TICK)
            {
                for (int i = 1; i <= NUM_VOICES; ++i)
                {
                    audio_stack.send_command(ReleaseNote(i));
                }
            }

            // 3. Reset loop
            if (timer >= RESET_TICK)
            {
                timer = 0;
            }*/
        }
        
    }
}
