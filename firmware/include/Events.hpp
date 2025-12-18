#pragma once

struct Event
{
    enum class Type {
        BUTTON_PRESS,
        BUTTON_RELEASE,
        ENCODER_LEFT,
        ENCODER_RIGHT
    };
    Type type;
    int ID;
};
