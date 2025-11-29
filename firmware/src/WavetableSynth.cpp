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
        if (morph_counter > 1024)
        {
            //morph_counter -= buffer.buffsize;
            morphdir = -1;
        }
        if (morph_counter < 0)
        {
            morphdir = 1;
        }
        
        for (size_t j = 0; j < voices; j++)
        {
            s_counters[j] += s_inc + j * detune * m_inc;
            if (s_counters[j] > 1024)
            {
                s_counters[j] -= 1024;
            }
            
        }

        auto cnt = s_counters[0];

        if (use_fm)
        {
            cnt = fmshiftCounter(0);
        }
        
        
        s_counter += s_inc;
        if (s_counter > 1024)
        {
            s_counter -= 1024;
        }
        float_t amp = 1-  morph_counter / buffer.buffsize;
        buffer.write16bit(i, sampleTableLinear(table, cnt) / 3, AudioBuffer::Mode::MONO);
    }
}

WavetableSynth::WavetableSynth()
{
    s_counter = 0;
    morph_counter = 0;
    morphdir = 1;
    freq = 440;
    detune = 5;
    voices = 2;
    wt_index = 0;
    k1 = 1;
    a = 0.2;
    use_fm = false;
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
