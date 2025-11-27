#include "WavetableSynth.hpp"

void WavetableSynth::audioCallback(AudioBuffer buffer)
{
    for (size_t i = 0; i < buffer.buffsize; i++)
    {
        buffer.write16bit(i, sine(440, buffer.maxamp / 2, buffer.SPS), AudioBuffer::Mode::MONO);
    }
}

WavetableSynth::WavetableSynth()
{
}

WavetableSynth::~WavetableSynth()
{
}
