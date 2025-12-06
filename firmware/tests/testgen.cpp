#include <iostream>
#include <SynthCore/Engine.hpp>
#include <format>

void printBuffer(const AudioBuffer &buffer)
{
    for (size_t i = 0; i < buffer.buffsize; i++)
    {
        auto tmp = buffer.buffer[i*2];
        auto val = static_cast<int16_t> (tmp >> 16);
        std::cout << std::format("{:10}", val) << std::endl;
    }   
}

void printSineWave()
{
    Synth::SynthEngine engine;
    //engine.initOneSineTest();
    //engine.initSineAndAmpTest();
    //engine.initSinFMModTest();
    //engine.initSinModAmpTest();
    engine.initWTTest();
    std::array<uint32_t, 2048> mock{};
    AudioBuffer buffer;
    buffer.buffer = mock.begin();
    buffer.buffsize = 1024;
    engine.audioCallback(buffer);
    printBuffer(buffer);
}

int main()
{
    printSineWave();

    return 0;
}

