#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"

#include "blink.pio.h"

void blink_program_init(PIO pio, uint sm, uint offset, uint pinl, uint pinclk)
{
    pio_sm_config c = blink_program_get_default_config(offset);
    pio_gpio_init(pio, pinl);
    pio_gpio_init(pio, pinclk);
    pio_sm_set_consecutive_pindirs(pio, sm, pinl, 1, true);
    pio_sm_set_consecutive_pindirs(pio, sm, pinclk, 1, true);

    sm_config_set_set_pins(&c, pinl, 1);
    sm_config_set_sideset_pins(&c, pinclk);
    sm_config_set_clkdiv_int_frac8(&c, 10, 1);

    pio_sm_init(pio, sm, offset, &c);
}

void test1(PIO pio, uint sm, uint offset, uint pinlatch, uint pinclk) {
    blink_program_init(pio, sm, offset, pinlatch, pinclk);
    pio_sm_set_enabled(pio, sm, true);

    //printf("Blinking pin %d at %d Hz\n", pin, freq);

    //pio->txf[sm] = (125000000 / (2 * freq)) - 3;

    
}

int main()
{
    stdio_init_all();

    // PIO Blinking example
    PIO pio;
    uint sm;
    uint offset = pio_add_program(pio, &blink_program);
    int pinlatch = 14;
    int pinclk = 15;

    bool success = pio_claim_free_sm_and_add_program(&blink_program, &pio, &sm, &offset);
    printf("Loaded program at %d\n", offset);
    
    test1(pio, 0, offset, pinlatch, pinclk);

    while (true) {
        printf("Hello, world!\n");
        sleep_ms(1000);
    }
}
