#include "SignalGenerator.h"
#include <string>
#include <algorithm>
using namespace std;
Generator::Generator(float freq, int samplerate, SType t) : frequency{freq}, sampleRate{samplerate}, type{t} {}
Generator::~Generator() { reset(); }
void Generator::add(float duration, function<double(int sampleIndex)> waveformFormula, std::vector<float>& buffer) {
    size_t samples = static_cast<size_t>(duration * sampleRate);
    std::vector<float> buff(samples);
    const size_t rampSamples = envelopeSamples(samples);
    for (size_t i{0}; i < samples; ++i) {
        float sample = static_cast<float>(waveformFormula(static_cast<int>(i)));
        float envelope = 1.0f;
        if (i < rampSamples)
            envelope = static_cast<float>(i) / static_cast<float>(rampSamples);
        else if (i >= samples - rampSamples)
            envelope = static_cast<float>(samples - 1 - i) / static_cast<float>(rampSamples);
        envelope = std::clamp(envelope, 0.0f, 1.0f);
        buff[i] = sample * envelope;
        advancePhase(1);
    }
    smoothEdges(buff);
    buffer.insert(buffer.end(), buff.begin(), buff.end());
}

void Generator::addSineWave(float duration, std::vector<float>& buffer) {
    add(duration, [this](int) { return this->sineWave(); }, buffer);
}
void Generator::addSquareWave(float duration, std::vector<float>& buffer) {
    add(duration, [this](int) { return this->squareWave(); }, buffer);
}
void Generator::addTriangleWave(float duration, std::vector<float>& buffer) {
        add(duration, [this](int) { return this->triangleWave(); }, buffer);
}
void Generator::addSawtoothWave(float duration, std::vector<float>& buffer) {
        add(duration, [this](int) { return this->sawtoothWave(); }, buffer);
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
    const size_t samples = static_cast<size_t>(duration * sampleRate);
    buffer.insert(buffer.end(), samples, 0.0f);
    advancePhase(samples);
}
void Generator::reset() {
    buffer.clear();
    phase = 0.0;
}

void Generator::advancePhase(size_t samples)
{
    const double phaseIncrement = 2.0 * pi * frequency / static_cast<double>(sampleRate);
    phase += phaseIncrement * static_cast<double>(samples);
    phase = std::fmod(phase, 2.0 * pi);
    if (phase < 0.0)
        phase += 2.0 * pi;
}

size_t Generator::envelopeSamples(size_t totalSamples) const
{
    const double rampSeconds = [this]() {
        switch (type)
        {
        case Sine:
            return 0.008;
        case Triangle:
            return 0.006;
        case Square:
        case Sawtooth:
        default:
            return 0.004;
        }
    }();

    const size_t requestedSamples = static_cast<size_t>(rampSeconds * sampleRate);
    return std::min(totalSamples / 2, std::max<size_t>(1, requestedSamples));
}

void Generator::smoothEdges(std::vector<float>& buffer) const
{
    double smoothingFactor = 0.0;
    switch (type)
    {
    case Sine:
        smoothingFactor = 0.0;
        break;
    case Triangle:
        smoothingFactor = 0.08;
        break;
    case Square:
        smoothingFactor = 0.18;
        break;
    case Sawtooth:
        smoothingFactor = 0.14;
        break;
    default:
        smoothingFactor = 0.0;
        break;
    }

    if (smoothingFactor <= 0.0 || buffer.size() < 2)
        return;

    float previous = buffer.front();
    for (size_t i = 1; i < buffer.size(); ++i)
    {
        previous = static_cast<float>(previous + smoothingFactor * (buffer[i] - previous));
        buffer[i] = previous;
    }
}

