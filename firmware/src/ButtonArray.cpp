#include <ButtonArray.hpp>

void ButtonArray::button_array_program_init()
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

void ButtonArray::poll()
{
    pio_sm_clear_fifos(pio.pio, pio.sm);
}

ButtonArray::ButtonArray(uint datapin, uint clockpin, uint latchpin)
{
    pins.clock      = clockpin;
    pins.datain     = datapin;
    pins.latch      = latchpin;

    pio_claim_free_sm_and_add_program(&button_array_program, &pio.pio, &pio.sm, &pio.offset);

    button_array_program_init();
}

ButtonArray::~ButtonArray()
{
    pio_sm_clear_fifos(pio.pio, pio.sm);
}