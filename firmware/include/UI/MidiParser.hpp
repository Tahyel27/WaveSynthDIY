#pragma once

#include <optional>
#include "AudioCommands.hpp"
#include "Events.hpp"

static const float MIDI_NOTES[128] = {
    8.1758f, 8.6620f, 9.1770f, 9.7227f, 10.3009f, 10.9134f, 11.5623f, 12.2499f,
    12.9783f, 13.7500f, 14.5676f, 15.4339f, 16.3516f, 17.3239f, 18.3540f, 19.4454f,
    20.6017f, 21.8268f, 23.1247f, 24.4997f, 25.9565f, 27.5000f, 29.1352f, 30.8677f,
    32.7032f, 34.6478f, 36.7081f, 38.8909f, 41.2034f, 43.6535f, 46.2493f, 48.9994f,
    51.9131f, 55.0000f, 58.2705f, 61.7354f, 65.4064f, 69.2957f, 73.4162f, 77.7817f,
    82.4069f, 87.3071f, 92.4986f, 97.9989f, 103.8262f, 110.0000f, 116.5409f, 123.4708f,
    130.8128f, 138.5913f, 146.8324f, 155.5635f, 164.8138f, 174.6141f, 184.9972f, 195.9977f,
    207.6523f, 220.0000f, 233.0819f, 246.9417f, 261.6256f, 277.1826f, 293.6648f, 311.1270f,
    329.6276f, 349.2282f, 369.9944f, 391.9954f, 415.3047f, 440.0000f, 466.1638f, 493.8833f,
    523.2511f, 554.3653f, 587.3295f, 622.2540f, 659.2551f, 698.4565f, 739.9888f, 783.9909f,
    830.6094f, 880.0000f, 932.3275f, 987.7666f, 1046.5023f, 1108.7305f, 1174.6591f, 1244.5079f,
    1318.5103f, 1396.9129f, 1479.9777f, 1567.9817f, 1661.2188f, 1760.0000f, 1864.6550f, 1975.5332f,
    2093.0045f, 2217.4610f, 2349.3181f, 2489.0159f, 2637.0205f, 2793.8259f, 2959.9554f, 3135.9635f,
    3322.4376f, 3520.0000f, 3729.3101f, 3951.0664f, 4186.0090f, 4434.9221f, 4698.6363f, 4978.0317f,
    5274.0410f, 5587.6517f, 5919.9108f, 6271.9270f, 6644.8752f, 7040.0000f, 7458.6202f, 7902.1328f,
    8372.0181f, 8869.8442f, 9397.2726f, 9956.0635f, 10548.082f, 11175.303f, 11839.822f, 12543.854f};

const char MSG_NOTE_ON = 9;
const char MSG_NOTE_OFF = 8;

std::optional<AudioCommand> parse_midi_message(MidiMessage msg)
{
    char message_class = msg.status >> 4; //the upper nibble
    char channel = msg.status & 15;       //the lower nibble

    if (message_class == MSG_NOTE_ON)
    {
        if (msg.data_2 != 0)
        {
            float velocity = (float)(msg.data_2) / 127.;
            return PressNote(msg.data_1, MIDI_NOTES[msg.data_1], velocity);
        }
        else
        {
            return ReleaseNote(msg.data_1);
        }
    }
    else if (message_class == MSG_NOTE_OFF)
    {
        return ReleaseNote(msg.data_1);
    }

    return std::nullopt;
}