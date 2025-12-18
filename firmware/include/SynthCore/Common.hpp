#pragma once

#include <cstdint>
#include <array>
#include <bitset>
#include <algorithm>

namespace Synth
{
    using float_t = float;

    constexpr size_t BUFFER_SIZE = 512;
    constexpr size_t CHUNK_SIZE = 32;
    constexpr size_t CHUNKS_PER_BUFFER = BUFFER_SIZE / CHUNK_SIZE;
    constexpr int VOICE_COUNT = 6;
    constexpr int MAX_GRAPH_NODES = 20;
    constexpr int maxamp = 32767 / 2.5;

    constexpr int SPS = 45045;
    constexpr float dt = 1 / static_cast<float>(SPS);

    class BufferPool
    {
    private:
        static constexpr size_t BUFFER_COUNT = 25;

        alignas(32) std::array<float_t, CHUNK_SIZE * BUFFER_COUNT> memory_pool;

        std::bitset<BUFFER_COUNT> claimed_buffers;

    public:
        BufferPool(/* args */) 
        {
            std::fill_n(memory_pool.begin(), memory_pool.size(), 0.0);
        }

        ~BufferPool() {};

        float_t *getBuffer(int bufferID)
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
            std::fill_n(memory_pool.begin(), BUFFER_COUNT * CHUNK_SIZE, 0);
        }

        void wipeAndFreeBuffers()
        {
            wipeBuffers();
            claimed_buffers.reset();
        }

        static size_t getSize() { return BUFFER_COUNT; };
    };

    inline float_t * prepareInBuffer(int bufferID, float_t v, BufferPool * pool, float_t * scratch)
    {
        if (bufferID == -1)
        {
            std::fill_n(scratch,CHUNK_SIZE,v);
            return scratch;
        }
        return pool->getBuffer(bufferID);
    }

} // namespace Synth
