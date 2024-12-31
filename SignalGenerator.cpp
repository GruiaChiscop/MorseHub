#include "SignalGenerator.h"
#include <string>
using namespace std;
Generator::Generator(float freq, int samplerate, SType t) : frequency{freq}, sampleRate{samplerate}, type{t} {}
Generator::~Generator() { reset(); }
void Generator::add(float duration, function<double(int sampleIndex)> waveformFormula, std::vector<float>& buffer) {
    size_t samples =static_cast<size_t>(duration*sampleRate);
    std::vector<float> buff(samples);
for(size_t i{0}; i<samples; ++i) {
    buff[i] = waveformFormula(i);
    }
buffer.insert(buffer.end(), buff.begin(), buff.end());
}

void Generator::addSineWave(float duration, std::vector<float>& buffer) {
    add(duration, [this](int sampleIndex) { return this->sineWave(sampleIndex); }, buffer);
}
void Generator::addSquareWave(float duration, std::vector<float>& buffer) {
    add(duration, [this](int sampleIndex) { return this->squareWave(sampleIndex); }, buffer);
}
void Generator::addTriangleWave(float duration, std::vector<float>& buffer) {
        add(duration, [this](int sampleIndex) { return this->triangleWave(sampleIndex); }, buffer);
}
void Generator::addSawtoothWave(float duration, std::vector<float>& buffer) {
        add(duration, [this](int sampleIndex) { return this->sawtoothWave(sampleIndex); }, buffer);
}
void Generator::add(float duration, std::vector<float>& buffer)
{
    switch (type)
    {
    case Sine:
        addSineWave(duration, buffer);
        break;
    case Square:
    addSquareWave(duration, buffer);
    break;
    case Triangle:
    addTriangleWave(duration, buffer);
    break;
    default:
    addSawtoothWave(duration, buffer);
        break;
    }
}

void Generator::addSilence(float duration, std::vector<float>& buffer) {
    buffer.insert(buffer.end(), duration*sampleRate, 0.0f);
}
void Generator::reset() { buffer.clear(); }

