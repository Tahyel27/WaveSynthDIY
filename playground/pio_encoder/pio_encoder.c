#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"

#include "encoder.pio.h"

int main()
{
    int new_value, delta, old_value = 0;
    int last_value = -1, last_delta = -1;

    // Base pin to connect the A phase of the encoder.
    // The B phase must be connected to the next pin
    const uint PIN_AB = 16;

    stdio_init_all();
    printf("Hello from quadrature encoder\n");

    PIO pio = pio0;
    const uint sm = 0;

    // we don't really need to keep the offset, as this program must be loaded
    // at offset 0
    pio_add_program(pio, &quadrature_encoder_program);
    encoder_program_init(pio, sm, PIN_AB, 0);

    while (1)
    {
        // note: thanks to two's complement arithmetic delta will always
        // be correct even when new_value wraps around MAXINT / MININT
        new_value = encoder_get_count(pio, sm);
        delta = new_value - old_value;
        old_value = new_value;

        if (new_value != last_value || delta != last_delta)
        {
            printf("position %8d, delta %6d\n", new_value, delta);
            last_value = new_value;
            last_delta = delta;
        }
        sleep_ms(100);
    }
}
