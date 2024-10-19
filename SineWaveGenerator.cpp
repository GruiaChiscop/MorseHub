#include "SineWaveGenerator.h"
using namespace Tonic;
static Synth synth; //our synthesizer
SineWaveGenerator::SineWaveGenerator(float seconds)
{
    duration = seconds;
    gen= SineWave().freq(m_frequency);
    synth.setOutputGen(gen);
}

vector<float> SineWaveGenerator::generate()
{
float buffer;
int frames = m_sampleRate*duration;
synth.fillBufferOfFloats(&buffer, frames, 2);
vector<float> finalBuffer(&buffer, &buffer + frames);
//return &buffer;
return finalBuffer;
}

void SineWaveGenerator::addSilence(vector<float>& outBuffer, float duration)
{
    int frames = static_cast<unsigned int>(m_sampleRate * duration);
vector<float> silenceSample(frames, 0.0f);
outBuffer.insert(outBuffer.end(), silenceSample.begin(), silenceSample.end());
}
