#include <stdio.h>
#include <cmath>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "AudioDevice.hpp"
#include "AnalogArray.hpp"
#include "ButtonArray.hpp"
#include "WavetableSynth.hpp"
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
    
    auto btarray = ButtonArray(16,17,18);

    auto wt = WavetableSynth();
    wt.setFM(false);
    wt.setFMmod(0.3);
    wt.setWT(2);
    wt.setVoices(3);
    wt.setDetune(1);
    wt.setFreq(525);

    std::array<float, 8> cmajor = {130.81, 146.83, 164.81, 174.61, 196, 220, 246.94, 261.63};
    int scaleindex = 0;

    auto adsr = ADSR();

    int SPS = device.getDeviceInfo().SPS;
    int bsize = device.getDeviceInfo().buffsize;
    float dt = static_cast<float>(bsize) / static_cast<float> (SPS);
    float timer = 0;

    adsr.setSource(&wt);

    int wtc = 0;
    int wtmax = 3;
    float detune = 0;
    bool detune_flag= false;

    float fmdiff = 0;
    float fmval = 0;
    float fmdecay = 2;

    device.setSource(&adsr);

    auto synth = sineSynth();

    auto analogs = AnalogArray(19, 20, 21, 26);
    float voltage = 0;
    float voltage2 = 0;

    device.initialize();

    //chors
    int C[] = {0, 2, 4};
    int F[] = {0, 3, 5};
    int G[] = {1, 4, 6};
    int * chord_pointer;
    chord_pointer = C;
    int chordindex = 0;

    while (true)
    {
        if(device.update())
        {
            voltage2 = analogs.readChannelVoltage(4);
            //wt.setFreq(voltage*200);
            voltage = analogs.readChannelVoltage(1);
            wt.setFMmod(fmval + fmdiff);

            btarray.poll();
            auto evopt = btarray.getEvent();
            while (evopt.has_value())
            {
                if (evopt->type == ButtonEvent::Type::PRESSED)
                {
                    if (evopt->button == 0)
                    {
                        wtc++;
                        if (wtc > wtmax)
                            wtc = 0;
                        
                        wt.setWT(wtc);
                        
                    }
                    if (evopt->button == 2)
                    {
                        detune += 0.5;
                    }
                    if (evopt->button == 1)
                    {
                        detune -= 0.5;
                    }
                    if (evopt->button == 3)
                    {
                        if (detune_flag)
                        {
                            detune_flag = false;
                        }
                        else
                        {
                            detune_flag = true;
                        }
                        
                    }
                    
                    if (evopt->button == 4)
                    {
                        wt.setFreq(cmajor[scaleindex]);
                        adsr.trigger();
                        scaleindex++;
                        if (scaleindex == 8)
                        {
                            scaleindex = 0;
                        }
                        fmval = (voltage-0.05) / 3;
                        fmdiff = 1;

                        printf("pressed\n");
                    }
                    if (evopt->button == 5)
                    {
                        chord_pointer = C;
                    }
                    if (evopt->button == 6)
                    {
                        chord_pointer = F;
                    }
                    if (evopt->button == 7)
                    {
                        chord_pointer = G;
                    }
                    
                    
                }
                evopt = btarray.getEvent();
            }
            /*if (btarray.isPressed(3))
            {
                wt.setDetune(detune);
            }
            else
            {
                wt.setDetune(0);
            }*/

            if (fmdiff > 0.01)
            {
                fmdiff -= fmdecay * dt;
            }
            else
            {
                fmdiff = 0;
            }

            timer += dt;
            if (timer > 0.3)
            {
                fmdiff = voltage;
                wt.setFreq(cmajor[chord_pointer[chordindex]]*2);
                if (detune_flag)
                {
                    wt.setDetune(voltage2 * 2);
                }
                else
                {
                    wt.setDetune(0);
                }
                
                //fmval = (voltage - 0.05) / 3;
                fmval = 0;
                adsr.trigger();
                timer = 0;
                chordindex++;
                if (chordindex > 2)
                {
                    chordindex = 0;
                }
                
            }
            
            
        }
    }
}
