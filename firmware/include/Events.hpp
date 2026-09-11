#pragma once

#include <optional>

enum class EventType
{
    BUTTON_PRESS,
    BUTTON_RELEASE,
    ENCODER_TURN,
    DIAL_CHANGE
};

struct EncoderTurn
{
    int encoder_id;
    int change;
};

union EventValue
{
    float f;
    int i;
    EncoderTurn encoder;
};

struct Event
{
    static Event button_press(int i)
    {
        auto ev = Event{};
        ev.m_type = EventType::BUTTON_PRESS;
        ev.m_value.i = i;
        return ev;
    }

    static Event button_release(int i)
    {
        auto ev = Event{};
        ev.m_type = EventType::BUTTON_RELEASE;
        ev.m_value.i = i;
        return ev;
    }

    static Event encoder_turn(int id, int change)
    {
        auto ev = Event{};
        ev.m_type = EventType::ENCODER_TURN;
        ev.m_value.encoder = EncoderTurn{.encoder_id = id, .change = change};
        return ev;
    }

    static Event dial_change(float f)
    {
        auto ev = Event{};
        ev.m_type = EventType::DIAL_CHANGE;
        ev.m_value.f = f;
        return ev;
    }

    bool is_type(EventType type)
    {
        return type == m_type;
    }

    int get_button_press()
    {
        return m_value.i;
    }

    int get_button_release()
    {
        return m_value.i;
    }

    EncoderTurn get_encoder_turn()
    {
        return m_value.encoder;
    }

    float get_dial_change()
    {
        return m_value.f;
    }
    
    EventType m_type;
    EventValue m_value;
};
