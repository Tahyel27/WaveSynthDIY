#pragma once

#include <array>

template <typename T, size_t N>
struct staticQueue
{
    std::array<T, N> data;

    size_t readIndex = 0;

    size_t writeIndex = 0;

    constexpr void push(const T &obj)
    {
        data[writeIndex] = obj;
        writeIndex++;
        if (writeIndex == N)
        {
            writeIndex = 0;
        }
    }

    constexpr void push(T &&obj)
    {
        data[writeIndex] = std::move(obj);
        writeIndex++;
        if (writeIndex == N)
        {
            writeIndex = 0;
        }
    }

    constexpr bool empty()
    {
        return writeIndex == readIndex;
    }

    constexpr T pop()
    {
        const T tmp = data[readIndex];
        if (readIndex != writeIndex)
        {
            readIndex++;
            if (readIndex == N)
            {
                readIndex = 0;
            }
        }
        return tmp;
    }
};