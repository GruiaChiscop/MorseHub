#include "RunnerAudioEffects.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <numbers>
#include <random>

#include "MorseGenerator.h"

namespace
{
std::mt19937& rng()
{
    static std::mt19937 generator(std::random_device{}());
    return generator;
}

struct Biquad
{
    float b0{};
    float b1{};
    float b2{};
    float a1{};
    float a2{};
    float z1{};
    float z2{};

    float process(float input)
    {
        const float output = input * b0 + z1;
        z1 = input * b1 + z2 - a1 * output;
        z2 = input * b2 - a2 * output;
        return output;
    }
};

struct ComplexBlock
{
    std::vector<float> re;
    std::vector<float> im;
};

std::vector<float> createKeyingEnvelope(const std::string& text, int speedCpm, int sampleRate)
{
    const float unit = 1.2f / static_cast<float>(speedCpm);
    const float dash = 3.0f * unit;
    const float charGap = 3.0f * unit;
    const float wordGap = 7.0f * unit;
    const std::size_t rampSamples = std::max<std::size_t>(1, static_cast<std::size_t>(sampleRate * 0.004));

    std::vector<float> envelope;

    auto appendLevel = [&](float durationSeconds, float level) {
        const std::size_t count = std::max<std::size_t>(1, static_cast<std::size_t>(durationSeconds * sampleRate));
        const std::size_t startIndex = envelope.size();
        envelope.resize(startIndex + count, level);
        if (level <= 0.0f)
            return;

        const std::size_t ramp = std::min(rampSamples, count / 2);
        for (std::size_t i = 0; i < ramp; ++i)
        {
            const float gain = static_cast<float>(i) / static_cast<float>(std::max<std::size_t>(1, ramp));
            envelope[startIndex + i] *= gain;
            envelope[startIndex + count - 1 - i] *= gain;
        }
    };

    std::string currentWord;
    auto flushWord = [&]() {
        if (currentWord.empty())
            return;
        for (char c : currentWord)
        {
            const char normalized = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            const auto it = MorseGenerator::characters.find(normalized);
            if (it == MorseGenerator::characters.end())
                continue;
            for (char symbol : it->second)
            {
                appendLevel(symbol == '-' ? dash : unit, 1.0f);
                appendLevel(unit, 0.0f);
            }
            appendLevel(charGap, 0.0f);
        }
        currentWord.clear();
    };

    for (char c : text)
    {
        if (c == ' ')
        {
            flushWord();
            appendLevel(wordGap, 0.0f);
        }
        else
            currentWord.push_back(c);
    }
    flushWord();
    appendLevel(wordGap, 0.0f);
    return envelope;
}

void applyQsbEnvelope(std::vector<float>& envelope, int sampleRate)
{
    if (envelope.empty())
        return;

    std::uniform_real_distribution<float> gainDistribution(0.25f, 1.0f);
    const std::size_t chunkSize = static_cast<std::size_t>(std::max(64, sampleRate / 45));
    float currentGain = gainDistribution(rng());

    for (std::size_t start = 0; start < envelope.size(); start += chunkSize)
    {
        const std::size_t end = std::min(envelope.size(), start + chunkSize);
        const float nextGain = gainDistribution(rng());
        const std::size_t count = std::max<std::size_t>(1, end - start);
        for (std::size_t i = 0; i < count; ++i)
        {
            const float t = static_cast<float>(i) / static_cast<float>(count);
            const float gain = currentGain + (nextGain - currentGain) * t;
            envelope[start + i] *= gain;
        }
        currentGain = nextGain;
    }
}

void mixEnvelopeAsStation(
    ComplexBlock& block,
    std::vector<float> envelope,
    int sampleRate,
    float bfoHz,
    float amplitude,
    bool enableQsb,
    bool enableFlutter)
{
    if (enableQsb)
        applyQsbEnvelope(envelope, sampleRate);

    std::uniform_real_distribution<float> flutterRateDistribution(8.0f, 18.0f);
    std::uniform_real_distribution<float> flutterDepthDistribution(0.0008f, 0.0030f);
    const float flutterRate = enableFlutter ? flutterRateDistribution(rng()) : 0.0f;
    const float flutterDepth = enableFlutter ? flutterDepthDistribution(rng()) : 0.0f;

    float phase = 0.0f;
    float flutterPhase = 0.0f;
    const float flutterPhaseStep = enableFlutter ? static_cast<float>((2.0 * std::numbers::pi * flutterRate) / sampleRate) : 0.0f;

    const std::size_t mixedSamples = std::min(envelope.size(), block.re.size());
    for (std::size_t i = 0; i < mixedSamples; ++i)
    {
        const float flutterOffset = enableFlutter ? (std::sin(flutterPhase) * bfoHz * flutterDepth) : 0.0f;
        const float phaseStep = static_cast<float>((2.0 * std::numbers::pi * (bfoHz + flutterOffset)) / sampleRate);
        const float stationSample = envelope[i] * amplitude;
        block.re[i] += stationSample * std::cos(phase);
        block.im[i] -= stationSample * std::sin(phase);
        phase += phaseStep;
        flutterPhase += flutterPhaseStep;
        if (phase > 2.0f * std::numbers::pi_v<float>)
            phase -= 2.0f * std::numbers::pi_v<float>;
        if (flutterPhase > 2.0f * std::numbers::pi_v<float>)
            flutterPhase -= 2.0f * std::numbers::pi_v<float>;
    }
}

void movingAveragePass(std::vector<float>& values, int points)
{
    if (values.empty() || points <= 1)
        return;

    std::vector<float> output(values.size(), 0.0f);
    double accumulator = 0.0;
    int count = 0;
    for (std::size_t i = 0; i < values.size(); ++i)
    {
        accumulator += values[i];
        ++count;
        if (count > points)
        {
            accumulator -= values[i - static_cast<std::size_t>(points)];
            count = points;
        }
        output[i] = static_cast<float>(accumulator / count);
    }
    values.swap(output);
}

void applyReceiverFilter(ComplexBlock& block, int sampleRate, float bandwidthHz)
{
    const int points = std::max(2, static_cast<int>(std::ceil(0.7f * sampleRate / std::max(80.0f, bandwidthHz))));
    for (int pass = 0; pass < 3; ++pass)
    {
        movingAveragePass(block.re, points);
        movingAveragePass(block.im, points);
    }
}

std::vector<float> modulateToAudio(const ComplexBlock& block, int sampleRate, float pitchHz)
{
    const std::size_t count = std::min(block.re.size(), block.im.size());
    std::vector<float> output(count, 0.0f);
    float phase = 0.0f;
    const float phaseStep = static_cast<float>((2.0 * std::numbers::pi * pitchHz) / sampleRate);
    for (std::size_t i = 0; i < count; ++i)
    {
        output[i] = block.re[i] * std::cos(phase) - block.im[i] * std::sin(phase);
        phase += phaseStep;
        if (phase > 2.0f * std::numbers::pi_v<float>)
            phase -= 2.0f * std::numbers::pi_v<float>;
    }
    return output;
}
}

std::vector<float> RunnerAudioEffects::renderMorseText(const std::string& text, int speedCpm, float pitch, SType signalType, int sampleRate)
{
    (void)signalType;
    std::vector<float> envelope = createKeyingEnvelope(text, speedCpm, sampleRate);
    ComplexBlock block;
    block.re.assign(envelope.size(), 0.0f);
    block.im.assign(envelope.size(), 0.0f);
    mixEnvelopeAsStation(block, std::move(envelope), sampleRate, pitch, 0.45f, false, false);
    return modulateToAudio(block, sampleRate, 0.0f);
}

std::vector<float> RunnerAudioEffects::renderIncomingSignal(
    const std::string& text,
    int speedCpm,
    const RunnerBandEffectsConfig& config,
    const std::vector<std::string>& qrmTexts,
    SType signalType)
{
    (void)signalType;

    std::vector<float> targetEnvelope = createKeyingEnvelope(text, speedCpm, config.sampleRate);
    if (targetEnvelope.empty())
        return {};

    const std::size_t paddingSamples = static_cast<std::size_t>(config.sampleRate * 0.75f);
    targetEnvelope.resize(targetEnvelope.size() + paddingSamples, 0.0f);

    ComplexBlock block;
    block.re.assign(targetEnvelope.size(), 0.0f);
    block.im.assign(targetEnvelope.size(), 0.0f);

    std::uniform_real_distribution<float> baseNoise(-0.08f, 0.08f);
    for (std::size_t i = 0; i < block.re.size(); ++i)
    {
        block.re[i] = 0.55f * baseNoise(rng());
        block.im[i] = 0.55f * baseNoise(rng());
    }

    mixEnvelopeAsStation(
        block,
        std::move(targetEnvelope),
        config.sampleRate,
        0.0f,
        0.62f,
        config.enableQsb,
        config.enableFlutter);

    if (config.enableQrn)
    {
        std::uniform_real_distribution<float> probabilityDistribution(0.0f, 1.0f);
        std::uniform_real_distribution<float> amplitudeDistribution(-0.85f, 0.85f);
        for (std::size_t i = 0; i < block.re.size(); ++i)
        {
            if (probabilityDistribution(rng()) < 0.004f)
            {
                block.re[i] += amplitudeDistribution(rng());
                block.im[i] += amplitudeDistribution(rng());
            }
        }
    }

    if (config.enableQrm)
    {
        std::uniform_real_distribution<float> offsetDistribution(-config.bandwidthHz * 0.85f, config.bandwidthHz * 0.85f);
        std::uniform_int_distribution<int> speedOffsetDistribution(-15, 25);
        std::uniform_real_distribution<float> amplitudeDistribution(0.18f, 0.33f);

        for (const std::string& qrmText : qrmTexts)
        {
            if (qrmText.empty())
                continue;

            std::vector<float> qrmEnvelope = createKeyingEnvelope(
                qrmText,
                std::max(80, speedCpm + speedOffsetDistribution(rng())),
                config.sampleRate);
            qrmEnvelope.resize(block.re.size(), 0.0f);
            mixEnvelopeAsStation(
                block,
                std::move(qrmEnvelope),
                config.sampleRate,
                offsetDistribution(rng()),
                amplitudeDistribution(rng()),
                config.enableQsb,
                config.enableFlutter);
        }
    }

    applyReceiverFilter(block, config.sampleRate, config.bandwidthHz);
    std::vector<float> output = modulateToAudio(block, config.sampleRate, config.centerPitchHz);
    applyAgc(output, config.sampleRate);
    return output;
}

void RunnerAudioEffects::applyBackgroundStatic(std::vector<float>& buffer)
{
    if (buffer.empty())
        return;

    std::uniform_real_distribution<float> noiseDistribution(-0.12f, 0.12f);
    float low1 = 0.0f;
    float low2 = 0.0f;
    for (float& sample : buffer)
    {
        const float white = noiseDistribution(rng());
        low1 = 0.985f * low1 + 0.015f * white;
        low2 = 0.92f * low2 + 0.08f * white;
        const float hiss = white - low2;
        const float body = (low2 - low1) * 0.55f;
        const float rumble = low1 * 0.12f;
        sample = std::clamp(sample + hiss * 0.45f + body + rumble, -1.0f, 1.0f);
    }
}

std::vector<float> RunnerAudioEffects::createStationBackground(int sampleRate, double seconds, bool enableQrn, bool enableFlutter)
{
    const std::size_t samples = static_cast<std::size_t>(std::max(1.0, seconds) * sampleRate);
    ComplexBlock block;
    block.re.assign(samples, 0.0f);
    block.im.assign(samples, 0.0f);

    std::uniform_real_distribution<float> baseNoise(-0.09f, 0.09f);
    for (std::size_t i = 0; i < samples; ++i)
    {
        block.re[i] = 0.6f * baseNoise(rng());
        block.im[i] = 0.6f * baseNoise(rng());
    }

    if (enableQrn)
    {
        std::uniform_real_distribution<float> probabilityDistribution(0.0f, 1.0f);
        std::uniform_real_distribution<float> amplitudeDistribution(-0.7f, 0.7f);
        for (std::size_t i = 0; i < samples; ++i)
        {
            if (probabilityDistribution(rng()) < 0.01f)
            {
                block.re[i] += amplitudeDistribution(rng());
                block.im[i] += amplitudeDistribution(rng());
            }
        }
    }

    applyReceiverFilter(block, sampleRate, 500.0f);
    std::vector<float> output = modulateToAudio(block, sampleRate, 600.0f);
    if (enableFlutter)
        applyFlutter(output, sampleRate);
    applyAgc(output, sampleRate);
    return output;
}

void RunnerAudioEffects::applyQsb(std::vector<float>& buffer, int sampleRate)
{
    applyQsbEnvelope(buffer, sampleRate);
}

void RunnerAudioEffects::applyQrn(std::vector<float>& buffer)
{
    if (buffer.empty())
        return;

    std::uniform_real_distribution<float> probabilityDistribution(0.0f, 1.0f);
    std::uniform_real_distribution<float> amplitudeDistribution(-1.0f, 1.0f);
    for (float& sample : buffer)
    {
        if (probabilityDistribution(rng()) < 0.004f)
            sample = std::clamp(sample + amplitudeDistribution(rng()) * 0.8f, -1.0f, 1.0f);
    }
}

void RunnerAudioEffects::applyFlutter(std::vector<float>& buffer, int sampleRate)
{
    if (buffer.empty())
        return;

    std::uniform_real_distribution<float> rateDistribution(8.0f, 18.0f);
    std::uniform_real_distribution<float> depthDistribution(0.06f, 0.14f);
    const float flutterRate = rateDistribution(rng());
    const float flutterDepth = depthDistribution(rng());
    const float phaseStep = static_cast<float>((2.0 * std::numbers::pi * flutterRate) / sampleRate);

    float phase = 0.0f;
    for (float& sample : buffer)
    {
        const float wobble = 1.0f + flutterDepth * std::sin(phase);
        sample = std::clamp(sample * wobble, -1.0f, 1.0f);
        phase += phaseStep;
    }
}

void RunnerAudioEffects::applyReceiverBandwidth(std::vector<float>& buffer, int sampleRate, float centerPitchHz, float bandwidthHz)
{
    if (buffer.empty() || sampleRate <= 0)
        return;

    const float safeCenter = std::clamp(centerPitchHz, 120.0f, static_cast<float>(sampleRate) * 0.45f);
    const float safeBandwidth = std::clamp(bandwidthHz, 120.0f, safeCenter * 1.5f);
    const float q = std::max(0.25f, safeCenter / safeBandwidth);
    const float omega = 2.0f * std::numbers::pi_v<float> * safeCenter / static_cast<float>(sampleRate);
    const float alpha = std::sin(omega) / (2.0f * q);
    const float cosOmega = std::cos(omega);
    const float a0 = 1.0f + alpha;

    Biquad filter;
    filter.b0 = alpha / a0;
    filter.b1 = 0.0f;
    filter.b2 = -alpha / a0;
    filter.a1 = (-2.0f * cosOmega) / a0;
    filter.a2 = (1.0f - alpha) / a0;

    for (float& sample : buffer)
        sample = filter.process(sample);
}

void RunnerAudioEffects::applyAgc(std::vector<float>& buffer, int sampleRate)
{
    if (buffer.empty() || sampleRate <= 0)
        return;

    const float targetLevel = 0.22f;
    const float minGain = 0.35f;
    const float maxGain = 4.0f;
    const float attackCoeff = std::exp(-1.0f / std::max(1.0f, sampleRate * 0.005f));
    const float releaseCoeff = std::exp(-1.0f / std::max(1.0f, sampleRate * 0.08f));

    float envelope = 0.0f;
    float gain = 1.0f;
    for (float& sample : buffer)
    {
        const float level = std::fabs(sample);
        const float coeff = level > envelope ? attackCoeff : releaseCoeff;
        envelope = coeff * envelope + (1.0f - coeff) * level;
        const float desiredGain = std::clamp(targetLevel / std::max(0.02f, envelope), minGain, maxGain);
        gain = 0.97f * gain + 0.03f * desiredGain;
        sample = std::clamp(sample * gain, -1.0f, 1.0f);
    }
}

void RunnerAudioEffects::mixQrm(std::vector<float>& buffer, const std::vector<float>& interference, std::size_t offsetSamples)
{
    if (buffer.empty() || interference.empty() || offsetSamples >= buffer.size())
        return;

    const std::size_t mixedSamples = std::min(buffer.size() - offsetSamples, interference.size());
    for (std::size_t i = 0; i < mixedSamples; ++i)
        buffer[offsetSamples + i] = std::clamp(buffer[offsetSamples + i] + interference[i] * 0.38f, -1.0f, 1.0f);
}

std::vector<float> RunnerAudioEffects::applyIncomingEffects(
    std::vector<float> baseBuffer,
    const RunnerBandEffectsConfig& config,
    const std::vector<std::string>& qrmTexts,
    int qrmSpeedCpm,
    float qrmPitch,
    SType qrmSignalType)
{
    (void)qrmSignalType;
    if (baseBuffer.empty())
        return {};

    std::vector<float> output = baseBuffer;
    applyBackgroundStatic(output);
    if (config.enableQrn)
        applyQrn(output);
    if (config.enableFlutter)
        applyFlutter(output, config.sampleRate);

    if (config.enableQrm)
    {
        for (const std::string& qrmText : qrmTexts)
        {
            std::vector<float> interference = renderMorseText(
                qrmText,
                qrmSpeedCpm,
                qrmPitch,
                qrmSignalType,
                config.sampleRate);
            mixQrm(output, interference);
        }
    }

    applyReceiverBandwidth(output, config.sampleRate, config.centerPitchHz, config.bandwidthHz);
    applyAgc(output, config.sampleRate);
    return output;
}
