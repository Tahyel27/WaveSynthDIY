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
    };
    Pins pins;

    AnalogArray(uint A, uint B, uint C);
    ~AnalogArray(){};
private:
};