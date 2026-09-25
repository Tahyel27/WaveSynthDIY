#pragma once

#include <cmath>
#include <variant>
#include <optional>

#include "../AudioCommands.hpp"

struct EncoderMapLinear
{
    float min_value = 0.0f;
    float max_value = 1.0f;
    float delta = 0.01f;

    float current_value = 0.0f;

    EncoderMapLinear() = default;

    EncoderMapLinear(float min, float max, float del, float start = 0.0f) 
        : min_value(min), max_value(max), delta(del), current_value(start) {};

    float turn(int steps)
    {
        current_value += steps * delta;
        if (current_value > max_value)
            current_value = max_value;
        else if (current_value < min_value )
            current_value = min_value;
        
        return current_value;
    }
};

struct EncoderMapLog
{
    float min_value = 1.0f;
    float max_value = 10000.f;
    float delta = 1.05f;

    float current_value = 1.0f;

    EncoderMapLog() = default;

    EncoderMapLog(float min, float max, int steps, float start = 1.0f)
        : min_value(min), max_value(max), current_value(start) 
    {
        float range = max_value - min_value;
        delta = powf(range, 1. / (float)steps);
    };

    float turn(int steps)
    {
        if (steps >= 0)
        {
            current_value = current_value * powf(delta, (float)steps);
        }
        else
        {
            current_value = current_value / powf(delta, (float)(-steps));
        }

        if (current_value > max_value)
            current_value = max_value;
        else if (current_value < min_value)
            current_value = min_value;

        return current_value;
    }
};

struct EncoderMapDiscrete
{
    int m_offset = 0;
    int m_steps = 5;
    int m_turns_per_step = 2;

    int m_counter = 0;

    EncoderMapDiscrete() = default;

    EncoderMapDiscrete(int steps, int turns_per_step, int offset = 0) 
        : m_steps(steps), m_turns_per_step(turns_per_step),
          m_offset(offset), m_counter(0) {};

    int turn(int steps)
    {
        m_counter += steps;
        if (m_counter < 0) 
            m_counter = 0;
        else if (m_counter > m_steps * m_turns_per_step)
            m_counter = m_steps * m_turns_per_step;
        
        return m_counter / m_turns_per_step;
    }
};

using EncoderMap = std::variant<EncoderMapLinear, EncoderMapLog, EncoderMapDiscrete>;

//encoder banks are used to represent mappings from the input actions to the resulting audio changes
//holds the encoder map and information on how to translate it to an audio action
struct EncoderBankEntry
{
    EncoderMap mapping;
    int target_ext_register = -1;

    EncoderBankEntry() = default;
    EncoderBankEntry(EncoderMap map, int ext_register_target) 
        : mapping(map), target_ext_register(ext_register_target) {};

    //implements the mapping of an encoder to an audio command
    std::optional<AudioCommand> turn_and_get_command(int steps) 
    {
        if (steps == 0) return std::nullopt;
        if (target_ext_register == -1) return std::nullopt;

        auto command = SetExtRegister();

        command.register_ID = target_ext_register;

        if (auto * encoder = std::get_if<EncoderMapLinear>(&mapping))
        {
            command.val.f = encoder->turn(steps);
        }
        else if (auto * encoder = std::get_if<EncoderMapLog>(&mapping))
        {
            command.val.f = encoder->turn(steps);
        }
        else if (auto * encoder = std::get_if<EncoderMapDiscrete>(&mapping))
        {
            command.val.u = encoder->turn(steps);
        }

        return command;
    }
};