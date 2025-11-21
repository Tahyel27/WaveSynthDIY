#include <stdio.h>
#include <queue>
#include "pico/stdlib.h"
#include "hardware/pio.h"

#include "blink.pio.h"

int buttons_prev[32];


void blink_program_init(PIO pio, uint sm, uint offset, uint pinl, uint pinclk, uint pinin)
{
    pio_sm_config c = blink_program_get_default_config(offset);
    pio_gpio_init(pio, pinl);
    pio_gpio_init(pio, pinclk);
    pio_gpio_init(pio, pinin);

    pio_sm_set_consecutive_pindirs(pio, sm, pinl, 1, true);
    pio_sm_set_consecutive_pindirs(pio, sm, pinclk, 1, true);
    pio_sm_set_consecutive_pindirs(pio, sm, pinin, 1, false);

    sm_config_set_in_pins(&c, pinin);
    sm_config_set_set_pins(&c, pinl, 1);
    sm_config_set_sideset_pins(&c, pinclk);
    sm_config_set_clkdiv_int_frac8(&c, 3, 1);
    sm_config_set_in_shift(&c, false, false, 32); //so the closest pin is the first in the register/FIFO

    pio_sm_init(pio, sm, offset, &c);
}

void test1(PIO pio, uint sm, uint offset, uint pinlatch, uint pinclk) {
    blink_program_init(pio, sm, offset, pinlatch, pinclk, 13);
    pio_sm_set_enabled(pio, sm, true);

    //printf("Blinking pin %d at %d Hz\n", pin, freq);

    //pio->txf[sm] = (125000000 / (2 * freq)) - 3;


}

bool contains(int *arr, size_t size, int value) 
{
    for (size_t i = 0; i < size; i++)
    {
        if(arr[i] == value)
            return true;
    }
    return false;
    
}

void fill(int *arr, size_t size, int value)
{
    for (size_t i = 0; i < size; i++)
    {
        arr[i] = value;
    }
    
}

void copy(int *arr_to, int *arr_from, size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        arr_to[i] = arr_from[i];
    }
    
}

void test2(PIO pio, uint sm, uint offset, uint pinlatch, uint pinclk, uint pinin)
{
    blink_program_init(pio, sm, offset, pinlatch, pinclk, 13);
    pio_sm_set_enabled(pio, sm, true);

    for (size_t i = 0; i < 10000; i++)
    {
        pio_sm_clear_fifos(pio, sm);
        //sleep_us(2);
        uint32_t words[4];
        const uint32_t mask = 0x7FFFFFFF;
        int loop_count = 0;
        while (pio_sm_get_rx_fifo_level(pio, sm) < 4)
        {
            sleep_us(1);
            if (loop_count > 100)
            {
                printf("failed to fill fifo in 100 iterations\n");
                break;
            }
            loop_count++;
            
        }
        printf("filled FIFO in %i loops \n", loop_count);
        
        for (size_t i = 0; i < 4; i++)  //this setup will read 4 words, and we discard the first, because it is from the previous read
        {
            words[i] = pio_sm_get_blocking(pio, sm);
            //printf("%x ", word);
        }
        /*for (size_t w = 0; w < 4; w++) //code for readingg four words
        {
            for (size_t i = 0; i < 32; i++)
            {
                bool button = ((words[w] | mask) == 0xFFFFFFFF);
                words[w] = words[w] << 1;
                printf("%d ", button);
            }
            printf("\n");
        }*/
        int buttons[32];
        fill(buttons, 32, -1);

        int b_index = 0;
        for (size_t i = 0; i < 32; i++)
        {
            bool down = ((words[1] | mask) == 0xFFFFFFFF);
            words[1] = words[1] << 1;
            //printf("%d ", down);
            if (down)
            {
                if (!contains(buttons_prev, 32, i))
                {
                    printf("Button %i pressed\n",i);
                }
                buttons[b_index++] = i;
            }
            else
            {
                if (contains(buttons_prev, 32, i))
                {
                    printf("Button %i released\n", i);
                }
            }
        }
        copy(buttons_prev, buttons, 32);
        //printf("\n");

        //printf("\n");

        pio_sm_clear_fifos(pio, sm);
        //printf("%d\n", pio_sm_get_rx_fifo_level(pio, sm));

        sleep_ms(10); //slowed down so we are testing if we really get the latest state
    }
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
    int pinin = 13;

    bool success = pio_claim_free_sm_and_add_program(&blink_program, &pio, &sm, &offset);
    printf("Loaded program at %d\n", offset);

    fill(buttons_prev, 32, -1);
    
    //test1(pio, 0, offset, pinlatch, pinclk);
    test2(pio, sm, offset, pinlatch, pinclk, pinin);

    while (true) {
        printf("Hello, world!\n");
        sleep_ms(1000);
    }
}
