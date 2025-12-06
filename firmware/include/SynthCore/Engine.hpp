#include <array>
#include <bitset>
#include <algorithm>
#include "AudioInterface.hpp"

namespace Synth
{
    constexpr size_t BUFFER_SIZE = 1024;
    constexpr size_t CHUNK_SIZE = 128;
    constexpr size_t CHUNKS_PER_BUFFER = BUFFER_SIZE / CHUNK_SIZE;
    constexpr int VOICE_COUNT = 6;
 // namespace Synth

using float_t = float;

class BufferPool
{
private:

    static constexpr size_t BUFFER_COUNT = 25;    

    alignas(32) std::array<float_t, CHUNK_SIZE * BUFFER_COUNT> memory_pool;

    std::bitset<BUFFER_COUNT> claimed_buffers;
public:
    BufferPool(/* args */){};
    ~BufferPool(){};

    float_t * getBuffer(int bufferID)
    {
        return &memory_pool[bufferID * CHUNK_SIZE];
    }

    int claimBuffer()
    {
        for (int i = 0; i < BUFFER_COUNT; i++)
        {
            if (!claimed_buffers[i])
            {
                claimed_buffers[i] = true;
                return i;
            }
        }
        
        return -1;
    }

    void freeBuffer(int i)
    {
        claimed_buffers[i] = false;
    }

    void wipeBuffers()
    {
        std::fill_n(memory_pool.begin(),BUFFER_COUNT * CHUNK_SIZE,0);
    }

    void wipeAndFreeBuffers()
    {
        wipeBuffers();
        claimed_buffers.reset();
    }

    static size_t getSize(){return BUFFER_COUNT;};
};

struct VoiceBuffers
{
    std::array<float_t, BUFFER_SIZE*VOICE_COUNT> data;

    float_t * get(int voice)
    {
        return &data[BUFFER_SIZE * voice];
    }
};


class SynthEngine : public AudioSource
{
private:
    /* data */
public:
    SynthEngine(/* args */){};
    ~SynthEngine(){};

    virtual void audioCallback(AudioBuffer Buffer) override;
};

}