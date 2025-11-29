#include "WavetableSynth.hpp"

int WavetableSynth::getBand(float f)
{
    int b_index = 0;
    bool found = false;
    auto wavetable = wt_library[wt_index];
    for (int i = 0; i < wavetable.num; i++)
    {
        if (wavetable[i].f > f)
        {
            return i;
        }
        
    }
    return wavetable.num - 1;
}

void WavetableSynth::audioCallback(AudioBuffer buffer)
{
    auto table = wt_library[wt_index][band_index].data;

    s_inc = (freq * static_cast<float_t>( buffer.buffsize ))/ static_cast<float_t>(buffer.SPS);

    //also acts as correction for detune
    float_t m_inc = static_cast<float_t>( buffer.buffsize )/ static_cast<float_t>(buffer.SPS);

    for (size_t i = 0; i < buffer.buffsize; i++)
    {
        morph_counter += morphdir * m_inc * 2;
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
        buffer.write16bit(i, sampleTableLinear(table, s_counters[0]) / 2, AudioBuffer::Mode::MONO);
    }
}

WavetableSynth::WavetableSynth()
{
    s_counter = 0;
    morph_counter = 0;
    morphdir = 1;
    freq = 120;
    detune = 5;
    voices = 2;
    wt_index = 0;
    band_index = getBand(static_cast<float>(freq));
}

WavetableSynth::~WavetableSynth()
{
}

void WavetableSynth::setFreq(float f)
{
    freq = static_cast<float_t>(f);
    band_index = getBand(static_cast<float>(freq));
}
