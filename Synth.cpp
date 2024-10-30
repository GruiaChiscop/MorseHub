/* Synth.cpp, the synthesizer for morse code*/
#include "Synth.h"
Synth::Synth(Type t, float frequency, int samplerate) : m_frequency{frequency}, m_sampleRate{samplerate} {
if(t == SINE) { gen = Tonic::SineWave().freq(m_frequency); }
else if(t == SQUARE) { gen = Tonic::SquareWave().freq(m_frequency); }
else if(t == SAWTOOTH) gen = Tonic::SawtoothWave().freq(m_frequency);
else if(t == TRIANGLE) gen = Tonic::TriangleWave().freq(m_frequency);
Tonic::setSampleRate(m_sampleRate);
synth.setOutputGen(gen);
}
void Synth::addToBuffer(float seconds) {
    unsigned int frames = static_cast<unsigned int>(m_sampleRate*seconds);
    std::vector<float> buff(frames);
    synth.fillBufferOfFloats(buff.data(), buff.size(), 1);
    outBuffer.reserve(outBuffer.size()+frames);
    outBuffer.insert(outBuffer.end(), buff.begin(), buff.end());
}
void Synth::addSilence(float seconds) {
    unsigned int frames = static_cast<unsigned int>(m_sampleRate*seconds);
        outBuffer.reserve(outBuffer.size()+frames);
    outBuffer.insert(outBuffer.end(), frames, 0.0f);
}
void Synth::reset() {
    outBuffer.clear();
}