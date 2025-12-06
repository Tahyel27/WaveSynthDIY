#include <SynthCore/Components.hpp>
#include <cmath>

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

    for (size_t i = 0; i < CHUNK_SIZE; i++)
    {
        outbuffer[i] = 0;
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