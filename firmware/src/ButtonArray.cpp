#include <ButtonArray.hpp>

bool readbit(uint32_t word, uint i)
{
    const uint32_t mask = 0x7FFFFFFF; // 01111111

    return (word << i | mask) == 0xFFFFFFFF; // checks if the bit is high
}

std::optional<ButtonArray> ButtonArray::claim(uint datapin, uint clockpin, uint latchpin)
{
    auto button_array = ButtonArray();
    auto pio_opt = PioHandler::acquire(&button_array_program, button_array_program_get_default_config);

    if (!pio_opt.has_value()) return std::nullopt;
    button_array.pio = std::move(pio_opt.value());
    
    //sets the pins
    button_array.pins.datain = datapin;
    button_array.pins.clock = clockpin;
    button_array.pins.latch = latchpin;

    button_array.pio.set_in_pins(button_array.pins.datain);
    button_array.pio.set_set_pins(button_array.pins.latch);
    button_array.pio.set_sideset_pins(button_array.pins.clock);

    button_array.pio.set_in_shift(false, false, 32);
    button_array.pio.set_clkdiv_int_frac8(3, 1);

    button_array.pio.init();
    button_array.pio.set_enabled(true);
        
    return button_array;
}

bool ButtonArray::isPressed(int button)
{
    const uint32_t mask = 0x7FFFFFFF; // 01111111

    return (prev_state << button | mask) == 0xFFFFFFFF; //checks if the button is pressed
}

ButtonArray::~ButtonArray()
{

}

std::optional<EncoderArray> EncoderArray::claim(uint datapin, uint latchpin, uint clockpin)
{
    auto encoder_array = EncoderArray{};
    auto pio_opt = PioHandler::acquire(&button_array_program, button_array_program_get_default_config);

    if (!pio_opt.has_value())
        return std::nullopt;
    encoder_array.pio = std::move(pio_opt.value());

    encoder_array.pins.datain = datapin;
    encoder_array.pins.clock = clockpin;
    encoder_array.pins.latch = latchpin;

    encoder_array.populate_encoders();

    encoder_array.pio.set_in_pins(encoder_array.pins.datain);
    encoder_array.pio.set_set_pins(encoder_array.pins.latch);
    encoder_array.pio.set_sideset_pins(encoder_array.pins.clock);

    encoder_array.pio.set_in_shift(false, false, 32);
    encoder_array.pio.set_clkdiv_int_frac8(3, 1);

    encoder_array.pio.init();
    encoder_array.pio.set_enabled(true);

    return encoder_array;
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
        ev = Event{.m_type = EventType::ENCODER_TURN, -1};
        return true;
    }
    else if (rot == 1)
    {
        ev = Event{.m_type = EventType::ENCODER_TURN, 1};
        return true;
    }
    else
    {
        return false;
    }
    
}

uint32_t EncoderArray::poll()
{
    pio.clear_fifos();

    uint32_t word = pio.get_blocking();

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