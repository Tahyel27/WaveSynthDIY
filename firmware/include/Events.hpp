#pragma once

#include <optional>

enum class EventType
{
    BUTTON_PRESS,
    BUTTON_RELEASE,
    ENCODER_TURN,
    DIAL_CHANGE
};

union EventValue
{
    float f;
    int i;
};

struct Event
{
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

    int get_encoder_turn()
    {
        return m_value.i;
    }

    float get_dial_change()
    {
        return m_value.f;
    }
    
    EventType m_type;
    EventValue m_value;
};
