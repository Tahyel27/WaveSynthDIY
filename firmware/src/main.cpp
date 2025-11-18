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
    double t = double(info.chunk + info.sample) / double(45045);
    return sinegen(t,maxamp/5,440);
}

int main()
{
    stdio_init_all();

    auto device = AudioDevice(12,13,AudioDevice::DeviceMode::MONO,1);

    device.initialize();

    while (true)
    {
        if (GLOBAL_DEVICE_FLAG)
        {
            device.update();
            GLOBAL_DEVICE_FLAG = false;
        }
    }
}
