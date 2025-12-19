#include <ButtonArray.hpp>

bool readbit(uint32_t word, uint i)
{
    const uint32_t mask = ((uint32_t)0 - 1) << 31;
    return ((word << i) & mask);
}

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

void EncoderArray::init_prorgram()
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

int EncoderArray::read_encoder(int i, uint32_t word)
{
    int rot = 0;
    const bool A = encoders[i].A;
    const bool B = encoders[i].B;
    bool nA = readbit(word, encoders[i].pinA);
    bool nB = readbit(word, encoders[i].pinB);
    bool chA = (nA != encoders[i].A);
    bool chB = (nB != encoders[i].B);
    if (chA || chB)
    {
        if (encoders[i].state)
        {
            if (!nA || !nB) // state was true, something is false state->false
            {
                encoders[i].state = false;
            }
            else
            {
                encoders[i].state = true;
            }
        }
        else
        {
            if (nA || nB) // if any is true false->true
            {
                encoders[i].state = true;
            }
            else
            {
                encoders[i].state = false;
            }
        }

        if (nA != nB)
        {
            if (encoders[i].state) // true first A = 1, first B = -1
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
    encoders[i].A = nA;
    encoders[i].B = nB;
    return rot;
}

bool EncoderArray::pollEncoder(int i, Event &ev, uint32_t word)
{
    int rot = read_encoder(i, word);
    if (rot == -1)
    {
        ev = Event{Event::Type::ENCODER_LEFT, i};
        return true;
    }
    else if (rot == 1)
    {
        ev = Event{Event::Type::ENCODER_RIGHT, i};
        return true;
    }
    else
    {
        return false;
    }
    
}

uint32_t EncoderArray::poll()
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

void EncoderArray::populate_encoders()
{
    for (size_t i = 0; i < encoders.size(); i++)
    {
        encoders[i].pinA = 2*i;
        encoders[i].pinB = 2*i+1;
    }
    
}

EncoderArray::EncoderArray(uint datapin, uint latchpin, uint clockpin)
{
    pins.clock = clockpin;
    pins.datain = datapin;
    pins.latch = latchpin;

    populate_encoders();

    pio_claim_free_sm_and_add_program(&button_array_program, &pio.pio, &pio.sm, &pio.offset);

    init_prorgram();
}
