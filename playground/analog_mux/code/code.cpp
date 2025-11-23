#include <stdio.h>
#include "pico/stdlib.h"

struct pins
{
    uint a;  //MSB
    uint b;
    uint c;  //LSB
};

void initPins(pins pins)
{
    gpio_init(pins.a);
    gpio_init(pins.b);
    gpio_init(pins.c);

    gpio_set_dir(pins.a, true);
    gpio_set_dir(pins.b, true);
    gpio_set_dir(pins.c, true);
}


void output3bit(uint8_t value, pins pins)
{
    gpio_put(pins.a, value & 4);
    gpio_put(pins.b, value & 2);
    gpio_put(pins.c, value & 1);
}


int main()
{
    stdio_init_all();

    //first we want to output a number 4 on pins 19,20,21
    pins my_pins = {19, 20, 21};

    initPins(my_pins);
    

    while (true) {
        for (int i = 0; i < 8; i++)
        {
            output3bit(i, my_pins);
            sleep_ms(250);
        }

        sleep_ms(500);
    }
}
