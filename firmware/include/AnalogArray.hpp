#include "pico/stdlib.h"
#include "hardware/adc.h"

class AnalogArray
{
public:
    struct Pins
    {
        uint A;
        uint B;
        uint C;
        uint read;
    };
    Pins pins;

    AnalogArray(uint A, uint B, uint C, uint read);
    ~AnalogArray(){};

    uint16_t readChannel(uint channel); // Read a specific ADC channe

    float readChannelVoltage(uint channel); // Read a specific ADC channel as

    float readChannelVoltageStable(uint channel);
private:
    void write3bits(uint v);
};