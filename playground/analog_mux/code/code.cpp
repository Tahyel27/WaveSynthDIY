#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

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

float ADCtoFloat(uint16_t ADCvalue)
{
    return ADCvalue * (3.3f / (1 << 12));   
}


int main()
{
    stdio_init_all();

    //first we want to output a number 4 on pins 19,20,21
    pins my_pins = {19, 20, 21};
    uint areadpin = 26;

    initPins(my_pins);

    adc_init();
    adc_gpio_init(areadpin);
    adc_select_input(0);

    while (true) {
        /*for (int i = 0; i < 8; i++)
        {
            output3bit(i, my_pins);
            uint16_t read = adc_read();
            printf("Voltage: %f\n", ADCtoFloat(read));
            sleep_us(200);
        }*/


        //sleep_ms(500);
        //to check response time
        output3bit(0, my_pins);
        uint16_t read = adc_read();
        sleep_ms(100);
        output3bit(4, my_pins);
        sleep_us(6);
        read = adc_read();
        printf("Voltage: %f\n", ADCtoFloat(read));
        sleep_ms(100); // delay to ensure stability
    }
}
