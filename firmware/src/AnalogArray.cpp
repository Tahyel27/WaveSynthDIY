#include "AnalogArray.hpp"

AnalogArray::AnalogArray(uint A, uint B, uint C, uint read)
{
    pins.A = A;
    pins.B = B;
    pins.C = C;
    pins.read = read;

    gpio_init(pins.A);
    gpio_init(pins.B);
    gpio_init(pins.C);

    gpio_set_dir(pins.A, true);
    gpio_set_dir(pins.B, true);
    gpio_set_dir(pins.C, true);

    adc_init();
    adc_gpio_init(pins.read);
    uint channel = pins.read - 26;
    hard_assert(channel < 4);
    adc_select_input(channel);
}

uint16_t AnalogArray::readChannel(uint channel)
{
    write3bits(channel);
    sleep_us(6);
    return adc_read();
}

float AnalogArray::readChannelVoltage(uint channel)
{
    write3bits(channel);
    sleep_us(6);
    uint16_t read = adc_read();
    return read * (3.3f / (1 << 12));
}

float AnalogArray::readChannelVoltageStable(uint channel)
{
    write3bits(channel);
    sleep_us(6);
    unsigned int sum = 0;
    const int readcount = 15;
    for (size_t i = 0; i < readcount; i++)
    {
        sleep_us(2);
        sum += static_cast<unsigned int>(adc_read());
    }
    return 3.3f * static_cast<float>(sum)/(static_cast<float>(readcount) * (1 << 12));
}

void AnalogArray::write3bits(uint v)
{
    gpio_put(pins.C, v & 4);
    gpio_put(pins.B, v & 2);
    gpio_put(pins.A, v & 1);
}
