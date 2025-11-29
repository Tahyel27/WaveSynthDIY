//stuff that we always write
#include "wave_tables.hpp"

//wavetable data
const int16_t table1[3] = { 10, 20, 30};
const int16_t table2[3] = { 20, 30, 40};
const int16_t table3[3] = { 0, -20, 30};

//const WaveTable definitions
const WaveTableBand wavetable1[] = {
    {table1, 1024, 30}, //the actual wavetable, its length, the frequency where it should play
    {table2, 1024, 500},
    {table3, 1024, 800}
};

const WaveTable wt_library[] = {
    {wavetable1, 3} //pointer to single wavetalbe, then number of tables for that wavetables (since we need multiple due to aliasing)
};