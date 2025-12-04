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
    auto tablesize = wt_library[wt_index][band_index].length;

    s_inc = (freq * static_cast<float_t>( tablesize ))/ static_cast<float_t>(buffer.SPS);

    //also acts as correction for detune
    float_t m_inc = static_cast<float_t>( tablesize )/ static_cast<float_t>(buffer.SPS);

    auto voice_array = std::array<int16_t, 4>{0,0,0,0};
    for (int i = 0; i < voices; i++)
    {
        voice_array[i] = 1;
    }
    
    for (size_t i = 0; i < buffer.buffsize; i++)
    {
        morph_counter += morphdir * m_inc * 2;
        if (morph_counter > tablesize)
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
            if (s_counters[j] > tablesize)
            {
                s_counters[j] -= tablesize;
            }
            
        }

        auto cnt0 = s_counters[0];
        auto cnt1 = s_counters[1];
        auto cnt2 = s_counters[2];

        if (use_fm)
        {
            cnt0 = fmshiftCounter(0, tablesize);
            cnt1 = fmshiftCounter(1, tablesize);
            cnt2 = fmshiftCounter(2, tablesize);
        }
        
        float_t amp = 1-  morph_counter / buffer.buffsize;
        int16_t sample = voice_array[0] * sampleTableLinear(table, cnt0) / (voices + 1);
        sample += voice_array[1] * sampleTableLinear(table, cnt1) / (voices + 1);
        sample += voice_array[2] * sampleTableLinear(table, cnt2) / (voices + 1);
        
        buffer.write16bit(i, sample*2, AudioBuffer::Mode::MONO);
    }
}

WavetableSynth::WavetableSynth()
{
    morph_counter = 0;
    morphdir = 1;
    freq = 440;
    detune = 1;
    voices = 1;
    wt_index = 0;
    k1 = 1;
    a = 0.5;
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

void WavetableSynth::setWT(int wti)
{
    wt_index = wti;
    band_index = getBand(static_cast<float>(freq));
}
