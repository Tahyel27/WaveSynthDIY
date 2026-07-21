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
    constexpr size_t REGISTER_SIZE = 25;

    constexpr int SPS = 45045;
    constexpr float dt = 1 / static_cast<float>(SPS);

    using ScalarRegister = std::array<float_t, REGISTER_SIZE>;

    class BufferPool
    {
    private:

        alignas(32) std::array<float_t, CHUNK_SIZE * REGISTER_SIZE> memory_pool;

        std::bitset<REGISTER_SIZE> claimed_buffers;

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
            for (int i = 0; i < REGISTER_SIZE; i++)
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
            std::fill_n(memory_pool.begin(), REGISTER_SIZE * CHUNK_SIZE, 0);
        }

        void wipeAndFreeBuffers()
        {
            wipeBuffers();
            claimed_buffers.reset();
        }

        static size_t getSize() { return REGISTER_SIZE; };
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

    //a short buffer consisting of only two values
    class ShortBufferView
    {
        float_t * m_first;
        float_t * m_second;
    public:
        ShortBufferView(float_t * first, float_t * second) : m_first(first), m_second(second) {};

        float_t & first() 
        {
            return *m_first;
        }

        float_t & second()
        {
            return *m_second;
        }
    };

    // a pool of buffers consisting of only two floating point values, used for low frequency modulation to save perf
    class ShortBufferPool 
    {
        alignas(32) std::array<float_t, REGISTER_SIZE * 2> data;
    public:
        ShortBufferPool() 
        {
            std::fill(data.begin(), data.end(), 0.0);
        }

        ShortBufferView getBuffer(int ID) 
        {
            return ShortBufferView(&data[ID*2], &data[ID*2 + 1]);
        }
    };

    enum class OperandType : uint8_t 
    {
        SCALAR_REG,
        SHORTBUF_REG,
        BUFFER_REG,
        EXT_REG,
        VARIANT_REG,
        IMMEDIATE
    };

    struct Operand 
    {
        OperandType type;
        union 
        {
            uint16_t reg_index;
            float value;
        };
    };

    

} // namespace Synth
