#include <stdio.h>
#include <cmath>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "AudioDevice.hpp"

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
        samplecounter++;
        return amp*sin(f*t*M_PI);
    }
};

void writeAudioIntoDevice(AudioDevice &device, sineSynth &synth)
{
    const int maxamp = 32767 / 2.5;
    uint64_t SPS = device.getDeviceInfo().SPS;
    for (size_t i = 0; i < device.getDeviceInfo().buffsize; i++)
    {
        device.writeAudio(i,synth(440,maxamp,SPS),AudioDevice::ChannelMode::MONO);
    }
}

int main()
{
    stdio_init_all();

    sleep_ms(6000);

    auto irqHandler = IRQHandler::getIRQHandler();

    auto device = AudioDevice(12,13,AudioDevice::DeviceMode::STEREO,1,irqHandler,false);

    auto synth = sineSynth();

    device.initialize();

    while (true)
    {
        if(device.update())
        {
            writeAudioIntoDevice(device,synth);
        }
    }
}
