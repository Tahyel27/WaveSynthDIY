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

    int buttons_index = 0;
    std::array<int, MAX_BUTTONS> buttons;
    std::fill(buttons.begin(), buttons.end(), -1);

    for (size_t i = 0; i < 32; i++)
    {
        const uint32_t mask = 0x7FFFFFFF;
        bool down = ((word | mask) == 0xFFFFFFFF);
        word = word << 1;
        if (down)
        {
            if (std::find(buttons_prev.begin(),buttons_prev.end(), i) == buttons_prev.end())
            {
                pressedQueue.push(i);
            }    
            buttons[buttons_index++] = i;
        }
        else
        {
            if (std::find(buttons_prev.begin(),buttons_prev.end(), i ) != buttons_prev.end())
            {
                releasedQueue.push(i);
            }
        }
    }

    std::copy(buttons.begin(), buttons.end(), buttons_prev.begin());    
}

bool ButtonArray::isPressed(int button)
{
    if (std::find(buttons_prev.begin(),buttons_prev.end(),button) != buttons_prev.end())
    {
        return true;
    }
    else
    {
        return false;
    }
}

std::optional<ButtonEvent> ButtonArray::getEvent()
{
    if (!pressedQueue.empty())
    {
        int button = pressedQueue.front();
        pressedQueue.pop();
        return ButtonEvent{button,deviceID, ButtonEvent::Type::PRESSED};
    }
    else if (!releasedQueue.empty())
    {
        int button = releasedQueue.front();
        releasedQueue.pop();
        return ButtonEvent{button, deviceID, ButtonEvent::Type::RELEASED};
    }
    else
    {
        return std::nullopt;
    }
    
}

ButtonArray::ButtonArray(uint datapin, uint clockpin, uint latchpin)
{
    pins.clock      = clockpin;
    pins.datain     = datapin;
    pins.latch      = latchpin;

    pio_claim_free_sm_and_add_program(&button_array_program, &pio.pio, &pio.sm, &pio.offset);

    button_array_program_init();

    std::fill(buttons_prev.begin(),buttons_prev.end(),-1);
}

ButtonArray::~ButtonArray()
{
    pio_sm_clear_fifos(pio.pio, pio.sm);
}