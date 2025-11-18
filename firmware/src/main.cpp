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

int main()
{
    stdio_init_all();

    IRQHandler irqHandler;

    auto device = AudioDevice(12,13,AudioDevice::DeviceMode::STEREO,1,&irqHandler);

    device.initialize();

    while (true)
    {
        device.update();
    }
}
