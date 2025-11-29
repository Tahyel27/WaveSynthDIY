//stuff that we always write
#pragma once

#include <cstdint>

struct WaveTableBand
{
    const int16_t * data;
    uint32_t length;
    const float f;

    //array acces operator to get the data
    inline int16_t operator[](uint32_t index) const 
    {
        return data[index]; 
    }
    
};

struct WaveTable
{
    const WaveTableBand *b_ptr;
    uint32_t num;

    //returns a struct containing {data: pointer to start of wt data for given band, length: length of this data, f: recommended frequency to switch to this band}
    inline WaveTableBand operator[](uint32_t index) const 
    {
        return b_ptr[index];
    }
    
};

extern const WaveTable wt_library[];

//wavetables go here

extern const int16_t table1[3];
extern const int16_t table2[3];
extern const int16_t table3[3];

extern const WaveTableBand wavetable1[];
