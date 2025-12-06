#include <gtest/gtest.h>
#include <SynthCore/Engine.hpp>
#include "AudioInterface.hpp"

/* FIXTURE BOILERPLATE
class EngineTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Runs before EACH test
    }

    void TearDown() override
    {
        // Runs after EACH test
    }


};
*/
using namespace Synth;

class EngineTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        buffer = AudioBuffer{arr, 1024, 45045, 35200};
    }

    void TearDown() override
    {
        // Runs after EACH test
    }

    uint32_t arr[2048];

    SynthEngine en;
    AudioBuffer buffer;
};

class BufferPoolTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        
    }

    void TearDown() override
    {
        // Runs after EACH test
    }

    void fillUpWithIntegers()
    {
        float_t * ptr = pool.getBuffer(0);
        for (size_t i = 0; i < CHUNK_SIZE * pool.getSize(); i++)
        {
            ptr[i] = i;
        }
    }

    BufferPool pool;
    int firstClaimedBuffer;
    int secondClaimedBuffer;
    int thirdClaimedBuffer;
};

TEST_F(EngineTest, TestCallback)
{
    en.audioCallback(buffer);
    EXPECT_EQ(0, buffer.buffer[200]);
}

TEST_F(BufferPoolTest, TestSingleClaim)
{
    EXPECT_EQ(0,pool.claimBuffer());
}

TEST_F(BufferPoolTest, TestSecondClaim)
{
    auto first = pool.claimBuffer();
    EXPECT_EQ(1,pool.claimBuffer());
}

TEST_F(BufferPoolTest, TestBufferEditing)
{
    auto first = pool.claimBuffer();
    float_t * bufferptr = pool.getBuffer(first);
    bufferptr[2] = 2;
    EXPECT_FLOAT_EQ(pool.getBuffer(first)[2], 2);
}

TEST_F(BufferPoolTest, TestFreeing)
{
    auto first = pool.claimBuffer();
    auto second = pool.claimBuffer();
    pool.freeBuffer(first);
    auto third = pool.claimBuffer();
    EXPECT_EQ(first, third);
}

TEST_F(BufferPoolTest, TestCorrectBufferPositioning)
{
    auto N = pool.getSize();
    fillUpWithIntegers();
    for (size_t i = 0; i < N; i++)
    {
        EXPECT_EQ(pool.getBuffer(i)[CHUNK_SIZE - 1],CHUNK_SIZE*(i + 1)-1);
        EXPECT_EQ(pool.getBuffer(i)[0],CHUNK_SIZE*i);
    }
    
}

TEST_F(BufferPoolTest, WipeTest)
{
    auto N = pool.getSize();
    fillUpWithIntegers();
    pool.wipeBuffers();
    for (size_t i = 0; i < N; i++)
    {
        auto * ptr = pool.getBuffer(i);
        for (size_t j = 0; j < CHUNK_SIZE; j++)
        {
            EXPECT_EQ(ptr[i],0);
        }
        
    }
    
}