#include <SynthCore/Components.hpp>
#include "wavetable_data.hpp"
#include <cmath>

inline int getBandIndex(float f, int wt_index)
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

inline Synth::float_t sampleTableLinear(const int16_t *t, float_t x)
{
    const Synth::float_t mul = 3.05185e-5;
    int i = static_cast<int>(floor(x));
    Synth::float_t f = x - i;
    return static_cast<Synth::float_t>((1 - f) * t[i] + f * t[i + 1]) * mul;
}

void Synth::processWTOsc(WTOscData * data, BufferPool *pool, float_t *outbuffer)
{    
    std::array<float_t, CHUNK_SIZE> s_detune;
    std::array<float_t, CHUNK_SIZE> s_phaseDistort;
    std::array<float_t, CHUNK_SIZE> s_phaseDistMod;
    std::array<float_t, CHUNK_SIZE> s_freq;
    std::array<float_t, CHUNK_SIZE> s_morph;

    float_t * detune = prepareInBuffer(data->detune.bufID, data->detune.v, pool, &s_detune[0]);
    float_t * phaseDistort = prepareInBuffer(data->phaseDistort.bufID, data->phaseDistort.v, pool, &s_phaseDistort[0]);
    float_t * phaseDistMod = prepareInBuffer(data->phaseDistMod.bufID, data->phaseDistMod.v, pool, s_phaseDistMod.begin());
    float_t * freq = prepareInBuffer(data->freq.bufID, data->freq.v, pool, s_freq.begin());
    float_t * morph = prepareInBuffer(data->morph.bufID, data->morph.v, pool, s_morph.begin());

    int wt_index = data->wtIndex;
    int band_index = getBandIndex(freq[0], wt_index);

    auto table = wt_library[wt_index][band_index].data;
    auto tablesize = wt_library[wt_index][band_index].length;

    const float increment = static_cast<float> (tablesize) / static_cast<float> (SPS);

    int unison = data->unison;

    const std::array<float, 3> unisonFactors = {0, 1, -1};
    const std::array<float, 4> unisonAmps = {1, 1, 0.4, 0.3};

    for (size_t i = 0; i < CHUNK_SIZE; i++)
    {
        data->phaseCounterA += increment * freq[i];
        data->phaseCounterB += increment * (freq[i] + detune[i]);
        data->phaseCounterC += increment * (freq[i] - detune[i]);

        if (data->phaseCounterA > tablesize)
        {
            data->phaseCounterA -= static_cast<float>(tablesize);
        }

        if (data->phaseCounterB > tablesize)
        {
            data->phaseCounterB -= static_cast<float>(tablesize);
        }

        if (data->phaseCounterC > tablesize)
        {
            data->phaseCounterC -= static_cast<float>(tablesize);
        }

        float phA = data->phaseCounterA + phaseDistMod[i] * phaseDistort[i];
        float phB = data->phaseCounterB + phaseDistMod[i] * phaseDistort[i];
        float phC = data->phaseCounterC + phaseDistMod[i] * phaseDistort[i];

        phA = std::clamp(phA, 0.f, static_cast<float>(tablesize));
        phB = std::clamp(phB, 0.f, static_cast<float>(tablesize));
        phC = std::clamp(phC, 0.f, static_cast<float>(tablesize));

        outbuffer[i] = unisonAmps[unison] * sampleTableLinear(table, data->phaseCounterA);
        
        if (unison == 2)
        {
            outbuffer[i] += unisonAmps[unison] * sampleTableLinear(table, data->phaseCounterB);
        }
        else if(unison == 3)
        {
            outbuffer[i] += unisonAmps[unison] * sampleTableLinear(table, data->phaseCounterB);
            outbuffer[i] += unisonAmps[unison] * sampleTableLinear(table, data->phaseCounterC);
        }
    }
    
}

void Synth::processSineOsc(SineOscData * data, BufferPool *pool, float_t *outbuffer)
{
    std::array<float_t, CHUNK_SIZE> s_phaseDistort;
    std::array<float_t, CHUNK_SIZE> s_phaseDistMod;
    std::array<float_t, CHUNK_SIZE> s_freq;

    float_t *phaseDistort = prepareInBuffer(data->phaseDistort.bufID, data->phaseDistort.v, pool, s_phaseDistort.begin());
    float_t *phaseDistMod = prepareInBuffer(data->phaseDistMod.bufID, data->phaseDistMod.v, pool, s_phaseDistMod.begin());
    float_t *freq = prepareInBuffer(data->freq.bufID, data->freq.v, pool, s_freq.begin());

    float_t phaseCounter = data->phaseCounter;

    for (size_t i = 0; i < CHUNK_SIZE; i++)
    {
        phaseCounter += dt*freq[i];
        if (phaseCounter > 1)
        {
            phaseCounter -= 1;
        }

        outbuffer[i] = sinf(2*M_PI*(phaseCounter + phaseDistMod[i] * phaseDistort[i]));
    }

    data->phaseCounter = phaseCounter;
    
}

void Synth::processSawOsc(SawOscData * data, BufferPool *pool, float_t *outbuffer)
{

}

void Synth::processAmplifier(const AmplifierData &data, BufferPool *pool, float_t *outbuffer)
{
    std::array<float_t, CHUNK_SIZE> gainScratch;

    float_t * inGain = prepareInBuffer(data.amount.bufID, data.amount.v, pool, &gainScratch[0]);
    float_t * input = pool->getBuffer(data.input.bufID);

    for (size_t i = 0; i < CHUNK_SIZE; i++)
    {
        outbuffer[i] = input[i] * inGain[i];   
    }

}

void Synth::processADSR(Synth::ADSRData *data, Synth::float_t *outbuffer)
{
    auto state = data->state;

    float dec_floor = std::clamp(data->sustain, 0.f, 1.f);
    if (state == ADSRData::State::ATTACK)
    {
        for (size_t i = 0; i < CHUNK_SIZE; i++)
        {
            data->timer += dt;
            outbuffer[i] = std::clamp(data->timer / data->attack, 0.f, 1.f);
        }

        if (data->timer > data->attack)
        {
            data->state = ADSRData::State::HOLD;
        }
    }
    else if (state == ADSRData::State::HOLD)
    {
        for (size_t i = 0; i < CHUNK_SIZE; i++)
        {
            data->timer += dt;
            outbuffer[i] = 1;
        }

        if (data->timer > (data->attack + data->hold))
        {
            data->state = ADSRData::State::DECAY;
        }
    }
    else if (state == ADSRData::State::DECAY)
    {
        for (size_t i = 0; i < CHUNK_SIZE; i++)
        {
            data->timer += dt;
            outbuffer[i] = 1 + ((dec_floor - 1.f) / data->decay) * (data->timer - data->attack - data->hold);
        }

        if (data->timer > (data->attack + data->hold + data->decay))
        {
            if (data->sustain > 0)
            {
                data->state = ADSRData::State::SUSTAIN;
            }
            else
            {
                data->state = ADSRData::State::IDLE;
            }
        }        
    }
    else if (state == ADSRData::State::RELEASE)
    {
        if (data->timer >= data->release)
        {
            data->state = ADSRData::State::IDLE;
        }
        else{
            for (size_t i = 0; i < CHUNK_SIZE; i++)
            {
                data->timer += dt;
                float_t out = 1 - (1 / data->release) * data->timer;  
            }
        }
    }  
}

void Synth::processSVFLPData(Synth::SVFData *data, Synth::BufferPool *pool, Synth::float_t *outbuffer)
{
    std::array<float_t, CHUNK_SIZE> s_mod;
    
    float_t * cutoffMod = prepareInBuffer(data->modulation.bufID, data->modulation.v, pool, s_mod.begin());
    float_t * inp = pool->getBuffer(data->input.bufID);

    float_t &z1 = data->z1;
    float_t &z2 = data->z2;
    const float_t fcut = data->fcut;
    const float_t fenv = data->fenv;

    const float_t Q = data->Q;
    const float_t iSPS = 1.f / static_cast<float>(SPS);

    for (size_t i = 0; i < CHUNK_SIZE; i++)
    {
        float_t in = inp[i];

        float_t g = (fcut + fenv * cutoffMod[i]) * M_PI * iSPS;
        float_t d = 1/(1 + 2*Q*g + g*g);
        float_t BP = (g*(in-z2) + z1)*d;
        float_t v1 = BP - z1; z1 = BP + v1;
        float_t v2 = g*BP;
        float_t LP = v2 + z2; z2 = LP + v2;

        outbuffer[i] = LP;
        
    }
}

void Synth::processDelayData(DelayData *data, BufferPool *pool, float_t *outbuffer)
{
    float_t * inbuffer = pool->getBuffer(data->input.bufID);
    float_t gain = data->gain;
    float_t * delayLine = data->delayLine.begin();

    int &w_index = data->write_index;
    int r_index = w_index - data->delay;
    if (r_index < 0)
    {
        r_index += DelayData::LENGTH;
    }
    
    for (size_t i = 0; i < CHUNK_SIZE; i++)
    {
        float_t in = inbuffer[i];
        float_t out = in + delayLine[r_index];
        outbuffer[i] = out;
        delayLine[w_index] = out * gain;
        w_index++;
        if (w_index == DelayData::LENGTH)
        {
            w_index = 0;
        }
        r_index++;
        if (r_index == DelayData::LENGTH)
        {
            r_index = 0;
        }
    }
}