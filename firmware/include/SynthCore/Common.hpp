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
    constexpr size_t MAX_INSTRUCTION_COUNT = 20;

    constexpr int SPS = 45045;
    constexpr float dt = 1 / static_cast<float>(SPS);

    union RegisterData 
    {
        float_t f;
        uint32_t u;
    };

    using ScalarRegister = std::array<RegisterData, REGISTER_SIZE>;

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
        float_t m_current;
        float_t m_increment;

    public:
        ShortBufferView(float_t * first, float_t * second) : m_first(first), m_second(second), m_current(*first) {
            constexpr float_t increment = 1. / static_cast<float_t>(CHUNK_SIZE);
            m_increment = increment * (*m_second - *m_first);
        };

        float_t & first() 
        {
            return *m_first;
        }

        float_t & second()
        {
            return *m_second;
        }

        float_t next() 
        {
            auto tmp = m_current;
            m_current += m_increment;
            return tmp;
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
        NONE,
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
            RegisterData value;
        };

        static Operand None() 
        {
            return Operand{OperandType::NONE, {.reg_index = 0}};
        }

        static Operand ScalarReg(uint16_t index)
        {
            return Operand{OperandType::SCALAR_REG, {.reg_index = index}};
        }

        static Operand ShortBufReg(uint16_t index)
        {
            return Operand{OperandType::SHORTBUF_REG, {.reg_index = index}};
        }

        static Operand BufferReg(uint16_t index)
        {
            return Operand{OperandType::BUFFER_REG, {.reg_index = index}};
        }

        static Operand ExternalReg(uint16_t index)
        {
            return Operand{OperandType::EXT_REG, {.reg_index = index}};
        }

        static Operand Immediate_f(float_t f)
        {
            return Operand{OperandType::IMMEDIATE, {.value = {.f = f}}};
        }

        static Operand Immediate_u(uint32_t u)
        {
            return Operand{OperandType::IMMEDIATE, {.value = {.u = u}}};
        }
    };

    struct Context
    {
        alignas(32) std::array<RegisterData, REGISTER_SIZE> scalar_reg{};
        alignas(32) std::array<float_t, BUFFER_SIZE> master_output{};
        int current_chunk;

        ShortBufferPool *short_buf_pool;
        BufferPool *bufferPool;
        ScalarRegister *external_register;

        float_t * get_scalar(Operand &op) 
        {
            if (op.type == OperandType::SCALAR_REG) 
            {
                return &scalar_reg[op.reg_index].f;
            }
            else if (op.type == OperandType::EXT_REG)
            {
                return &(*external_register)[op.reg_index].f;
            }
            else if (op.type == OperandType::SHORTBUF_REG)
            {
                return &short_buf_pool->getBuffer(op.reg_index).first();
            }
            else if (op.type == OperandType::IMMEDIATE)
            {
                return &op.value.f;
            }
            else 
            {
                return nullptr;
            }
        }

        uint32_t * get_uint32(Operand &op)
        {
            if (op.type == OperandType::SCALAR_REG)
            {
                return &scalar_reg[op.reg_index].u;
            }
            else if (op.type == OperandType::EXT_REG)
            {
                return &(*external_register)[op.reg_index].u;
            }
            else if (op.type == OperandType::IMMEDIATE)
            {
                return &op.value.u;
            }
            else
            {
                return nullptr;
            }
        }

        float_t * get_buffer(Operand &op)
        {
            if (op.type == OperandType::BUFFER_REG)
            {
                return bufferPool->getBuffer(op.reg_index);
            }
            else
            {
                return nullptr;
            }
        }

        ShortBufferView get_short_buffer(Operand &op)
        {
            if (op.type == OperandType::SHORTBUF_REG)
            {
                return short_buf_pool->getBuffer(op.reg_index);
            }
            else if (op.type == OperandType::SCALAR_REG) 
            {
                auto ptr = get_scalar(op);
                return ShortBufferView{ptr, ptr};
            }
            else if (op.type == OperandType::IMMEDIATE)
            {
                return ShortBufferView{&op.value.f, &op.value.f};
            }
            else
            {
                return ShortBufferView{nullptr, nullptr};
            }

        }

        float_t * get_master()
        {
            return &master_output[CHUNK_SIZE * current_chunk];
        }

        void set_frequency(float_t freq)
        {
            //Sets the frequency of the sound, frequency is stored in scalar register 0
            scalar_reg[0].f = freq;
        }

        void set_velocity(float_t vel)
        {
            //sets the velocity of the sound, by convention it is scalar register 1
            scalar_reg[1].f = vel;
        }

        void set_gate(bool gate)
        {
            //sets the gate, by convention it is scalar register 2
            if(gate) scalar_reg[2].f = 1.f; 
            else scalar_reg[2].f = 0.f;
        }
    };

} // namespace Synth
