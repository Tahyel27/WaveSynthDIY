#include <SynthCore/Engine.hpp>

using namespace Synth;

void SynthEngine::audioCallback(AudioBuffer Buffer)
{
    for (size_t i = 0; i < Buffer.buffsize; i++)
    {
        Buffer.write16bit(i, 0, AudioBuffer::Mode::MONO);
    }
}