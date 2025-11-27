#include "WavetableSynth.hpp"

int16_t WavetableSynth::sampleLinear(float_t x)
{
    int i = static_cast<int>(round(x));
    float_t f = x - i;
    return (1 - f) * table1[i] + f * table1[i+1];
}

int16_t WavetableSynth::sampleLinearSecond(float_t x)
{
    int i = static_cast<int>(round(x));
    float_t f = x - i;
    return (1 - f) * table2[i] + f * table2[i + 1];
}

void WavetableSynth::audioCallback(AudioBuffer buffer)
{
    /*for (size_t i = 0; i < buffer.buffsize; i++)
    {
        buffer.write16bit(i, table1[i] / 4, AudioBuffer::Mode::MONO);
    }*/
    s_inc = (freq * static_cast<float_t>( buffer.buffsize ))/ static_cast<float_t>(buffer.SPS);

    //also acts as correction for detune
    float_t m_inc = static_cast<float_t>( buffer.buffsize )/ static_cast<float_t>(buffer.SPS);

    for (size_t i = 0; i < buffer.buffsize; i++)
    {
        morph_counter += morphdir * m_inc * 8;
        if (morph_counter > buffer.buffsize)
        {
            //morph_counter -= buffer.buffsize;
            morphdir = -1;
        }
        if (morph_counter < 0)
        {
            morphdir = 1;
        }
        
        for (size_t i = 0; i < voices; i++)
        {
            s_counters[i] += s_inc + i * detune * m_inc;
            if (s_counters[i] > buffer.buffsize)
            {
                s_counters[i] -= buffer.buffsize;
            }
            
        }
        
        
        s_counter += s_inc;
        if (s_counter > buffer.buffsize)
        {
            s_counter -= buffer.buffsize;
        }
        float_t amp = 1-  morph_counter / buffer.buffsize;
        buffer.write16bit(i, (amp * sampleLinear(s_counters[0]) + 1.2 * (1 - amp) * sampleLinearSecond(s_counters[1])) / 2, AudioBuffer::Mode::MONO);
    }
}

WavetableSynth::WavetableSynth()
{
    s_counter = 0;
    morph_counter = 0;
    morphdir = 1;
    freq = 70;
    detune = 5;
    voices = 2;
}

WavetableSynth::~WavetableSynth()
{
}

void WavetableSynth::setFreq(float f)
{
    freq = static_cast<float_t>(f);
}
