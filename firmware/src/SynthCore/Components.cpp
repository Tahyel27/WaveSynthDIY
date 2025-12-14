#include <SynthCore/Components.hpp>
#include "wavetable_data.hpp"
#include <cmath>

void Synth::processNode(NodeType type, int nodeID, Data &data, float_t *outbuffer, BufferPool *buffers)
{
    switch (type)
    {
    case NodeType::WTOSCILLATOR:
        processWTOsc(&data.WTOscArr[nodeID], buffers, outbuffer);
        break;
    case NodeType::AMPLIFIER:
        processAmplifier(data.AmplifierArr[nodeID], buffers, outbuffer);
        break;
    case NodeType::SINEOSCILLATOR:
        processSineOsc(&data.SineOscArr[nodeID], buffers, outbuffer);
        break;
    case NodeType::ADSR:
        processADSR(&data.ADSRArr[nodeID], outbuffer);
        break;
    case NodeType::SVFLP:
        processSVFLPData(&data.SVFArr[nodeID], buffers, outbuffer);
        break;
    default:
        break;
    }
}

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
    int i = static_cast<int>(x);
    Synth::float_t f = x - i;
    return static_cast<Synth::float_t>((1 - f) * t[i] + f * t[i + 1]) * mul;
}

static inline Synth::float_t sampleTableLinearFixed(const int16_t *t, uint32_t t10Q22)
{
    const int16_t A = t[t10Q22 >> 22];
    const int16_t B = t[(t10Q22 >> 22) + 1];
    const uint32_t max32bit = ((uint32_t)0 - 1);
    const uint32_t max16bit = max32bit >> 16;
    const float bit16tofl = 1. / static_cast<float>(max16bit);
    const float tableTo0float1 = bit16tofl * bit16tofl;

    const uint32_t angle = (max32bit >> 10) & t10Q22;

    const uint32_t inter = angle >> 6;

    const int32_t value = (uint32_t)A * (max16bit - inter) + (uint32_t)B * (inter);
    return static_cast<Synth::float_t>(value) * tableTo0float1;
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

    float_t tablesize_f = static_cast<float_t>(tablesize);
    const uint32_t phaseInrement_base = (1 << 22);
    const uint32_t max32bit = ((uint32_t)0 - 1);
    const float scale = 1.f / static_cast<float>((max32bit >> 10));
    const float phi_iAf = static_cast<float>(phaseInrement_base) * tablesize_f * freq[0] / SPS;
    const float phi_iBf = static_cast<float>(phaseInrement_base) * tablesize_f * (freq[0] + detune[0]) / SPS;
    const float phi_iCf = static_cast<float>(phaseInrement_base) * tablesize_f * (freq[0] - detune[0]) / SPS;
    const uint32_t phi_iA = static_cast<uint32_t>(phi_iAf);
    const uint32_t phi_iB = static_cast<uint32_t>(phi_iBf);
    const uint32_t phi_iC = static_cast<uint32_t>(phi_iCf);
    const float_t dist_base = static_cast<float>(phaseInrement_base) * tablesize_f * phaseDistMod[0];

    int unison = data->unison;

    const std::array<float, 3> unisonFactors = {0, 1, -1};
    const std::array<float, 4> unisonAmps = {1, 1, 0.4, 0.3};

    
    if (unison == 1)
    {
        for (size_t i = 0; i < CHUNK_SIZE; i++)
        {
            data->phA += phi_iA;
            float_t dist = static_cast<float>(phaseInrement_base) * tablesize_f * phaseDistMod[i] * phaseDistort[i];
            const uint32_t phiAdist = data->phA + static_cast<uint32_t>(dist);

            outbuffer[i] = sampleTableLinearFixed(table, phiAdist);
        }
        
    }
    else if (unison == 2)
    {
        for (size_t i = 0; i < CHUNK_SIZE; i++)
        {
            data->phA += phi_iA;
            data->phB += phi_iB;
            float_t dist = static_cast<float>(phaseInrement_base) * tablesize_f * phaseDistMod[i] * phaseDistort[i];
            const uint32_t phiAdist = data->phA + static_cast<uint32_t>(dist);
            const uint32_t phiBdist = data->phB + static_cast<uint32_t>(dist);

            outbuffer[i] = sampleTableLinearFixed(table, phiAdist) * 0.4;
            outbuffer[i] += sampleTableLinearFixed(table, phiBdist) * 0.4;
        }
    }
    else if (unison == 3)
    {
        for (size_t i = 0; i < CHUNK_SIZE; i++)
        {
            data->phA += phi_iA;
            data->phB += phi_iB;
            data->phC += phi_iC;
            float_t dist = dist_base * phaseDistort[i];
            const uint32_t phiAdist = data->phA + static_cast<uint32_t>(dist);
            const uint32_t phiBdist = data->phB + static_cast<uint32_t>(dist);
            const uint32_t phiCdist = data->phC + static_cast<uint32_t>(dist);

            const float_t outA = sampleTableLinearFixed(table, phiAdist);
            const float_t outB = sampleTableLinearFixed(table, phiBdist);
            const float_t outC= sampleTableLinearFixed(table, phiCdist);
            outbuffer[i] = 0.4*(outA + outB + outC);
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

    //float_t phaseCounter = data->phaseCounter;
    uint32_t phaseCounter = data->ph; 
    //const uint32_t f = static_cast<uint32_t>(freq[0] * 16.f) << (22 - 4);
    const uint32_t phaseInrement_base = (1 << 22);
    const int tablesize = 1024;
    const float phi_i = static_cast<float>(phaseInrement_base) * tablesize * freq[0] / SPS;
    const uint32_t phaseIncrement = static_cast<uint32_t> (phi_i);


    for (size_t i = 0; i < CHUNK_SIZE; i++)
    {
        /*phaseCounter += dt*freq[i];
        if (phaseCounter > 1)
        {
            phaseCounter -= 1;
        }*/
        phaseCounter += phaseIncrement;

        //outbuffer[i] = sinf(2*M_PI*(phaseCounter + phaseDistMod[i] * phaseDistort[i]));
        outbuffer[i] = sampleTableLinearFixed(wt_library[0][0].data, phaseCounter) * 4;
    }

    data->phaseCounter = phaseCounter;
    data->ph = phaseCounter;
    
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
    const float_t g = (fcut + fenv * cutoffMod[0]) * M_PI * iSPS;
    const float_t d = 1 / (1 + 2 * Q * g + g * g);

    for (size_t i = 0; i < CHUNK_SIZE; i++)
    {
        float_t in = inp[i];

        //float_t g = (fcut + fenv * cutoffMod[i]) * M_PI * iSPS;
        //float_t d = 1/(1 + 2*Q*g + g*g);
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

void Synth::globalDelay(DelayData *data, float_t *input, float_t *output, size_t bufferLength)
{
    float_t gain = data->gain;
    float_t *delayLine = data->delayLine.begin();

    int &w_index = data->write_index;
    int r_index = w_index - data->delay;
    if (r_index < 0)
    {
        r_index += DelayData::LENGTH;
    }

    for (size_t i = 0; i < bufferLength; i++)
    {
        float_t in = input[i];
        float_t out = in + delayLine[r_index];
        output[i] = out;
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