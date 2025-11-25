#include <stdio.h>
#include <cmath>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "AudioDevice.hpp"
#include "AnalogArray.hpp"

int16_t sinegen(double t, int amp, double f)
{
    return amp * sin(f * t * M_PI);
}

int16_t sample_callback(const AudioDevice::ChannelInfo &info)
{
    const int maxamp = 32767 / 2.5;
    double t = double(info.chunk + info.sample) / double(info.SPS);
    int16_t v = 0;
    if (info.chmode == AudioDevice::ChannelMode::LEFT)
    {
        v = sinegen(t, maxamp / 5, 340);
    }
    else
    {
        v = sinegen(t, maxamp / 5, 440);
    }
    return v;
}

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

    auto device = AudioDevice(12,13,AudioDevice::DeviceMode::STEREO,1,irqHandler,false);

    auto source = sineSource();

    device.setSource(&source);

    auto synth = sineSynth();

    //auto analogs = AnalogArray(19, 20, 21, 26);
    //float voltage;

    device.initialize();

    while (true)
    {
        if(device.update())
        {
            /*auto deviceInfo = device.getDeviceInfo();

            for (size_t i = 0; i < deviceInfo.buffsize; i++)
            {
                device.writeAudio(i, synth(440, deviceInfo.maxamp / 4, deviceInfo.SPS), AudioDevice::ChannelMode::MONO);
            }*/
        }
    }
}
