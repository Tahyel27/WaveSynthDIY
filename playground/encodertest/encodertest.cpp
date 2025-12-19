#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "button_array.pio.h"

struct Pins
{
    uint clock;
    uint latch;
    uint datain;
};

struct PioData
{
    PIO pio;
    uint sm;
    uint offset;
};

struct Encoder
{
    uint pinA;
    uint pinB;
    bool state = false;
    bool A;
    bool B;
};

const Pins pins = {3,4,5};

void button_array_program_init(PioData pio)
{
    pio_sm_config c = button_array_program_get_default_config(pio.offset);

    pio_gpio_init(pio.pio, pins.clock);
    pio_gpio_init(pio.pio, pins.datain);
    pio_gpio_init(pio.pio, pins.latch);

    pio_sm_set_consecutive_pindirs(pio.pio, pio.sm, pins.clock, 1, true);
    pio_sm_set_consecutive_pindirs(pio.pio, pio.sm, pins.latch, 1, true);
    pio_sm_set_consecutive_pindirs(pio.pio, pio.sm, pins.datain, 1, false);

    sm_config_set_in_pins(&c, pins.datain);
    sm_config_set_set_pins(&c, pins.latch, 1);
    sm_config_set_sideset_pins(&c, pins.clock);

    sm_config_set_clkdiv_int_frac8(&c, 3, 1);
    sm_config_set_in_shift(&c, false, false, 32); // so the closest pin is the first in the register/FIFO

    pio_sm_init(pio.pio, pio.sm, pio.offset, &c);

    pio_sm_set_enabled(pio.pio, pio.sm, true);
}

void printbinary(uint32_t n)
{
    for (int i = 31; i >= 0; i--)
        printf("%d", (n >> i) & 1);
    printf("\n");
}

bool readbit(uint32_t word, uint i)
{
    const uint32_t mask = ((uint32_t)0 - 1) << 31;
    return ((word << i) & mask);
}

int readEncoder(Encoder *enc, uint32_t word)
{
    int rot = 0;
    const bool A = enc->A;
    const bool B = enc->B;
    bool nA = readbit(word, enc->pinA);
    bool nB = readbit(word, enc->pinB);
    bool chA = (nA != enc->A);
    bool chB = (nB != enc->B);
    bool nstate = enc->state;
    if (chA || chB)
    {
        if (enc->state)
        {
            if (!nA || !nB) //state was true, something is false state->false
            {
                enc->state = false;
            }
            else
            {
                enc->state = true;
            }
        }
        else
        {
            if (nA || nB) // if any is true false->true
            {
                enc->state = true;
            }
            else
            {
                enc->state = false;
            }
        }
        
        
        if (nA != nB)
        {
            if (enc->state) //true first A = 1, first B = -1
            {
                if (nA)
                {
                    rot = 1;
                }
                else
                {
                    rot = -1;
                }
            }
            else
            {
                if (nA)
                {
                    rot = -1;
                }
                else
                {
                    rot = 1;
                }
            }   
        }
    }
    enc->A = nA;
    enc->B = nB;

    return rot;
}

uint32_t poll(PioData pio)
{
    pio_sm_clear_fifos(pio.pio, pio.sm);

    char loopcounter = 0;
    while (pio_sm_get_rx_fifo_level(pio.pio, pio.sm) < 2)
    {
        if (loopcounter > 50)
        {
            break;
        }
        loopcounter++;
    }
    uint32_t word = pio_sm_get_blocking(pio.pio, pio.sm);
    word = pio_sm_get_blocking(pio.pio, pio.sm);

    return word;
}

int main()
{
    stdio_init_all();

    PioData pio;
    pio_claim_free_sm_and_add_program(&button_array_program, &pio.pio, &pio.sm, &pio.offset);

    button_array_program_init(pio);

    Encoder enc;
    enc.pinA = 0; enc.pinB = 1;

    while (true) {
        //printf("Hello, world!\n");
        uint32_t w = poll(pio);
        printbinary(w);
        //printf("%d\n", readbit(w,0));
        int res = readEncoder(&enc, w);
        if (res)
        {
            printf("%d\n", res);
        }
        sleep_ms(12);
    }
}
