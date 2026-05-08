#pragma once

#include <string>
#include <vector>

#include "SignalGenerator.h"

struct RunnerBandEffectsConfig
{
    bool enableQrm{};
    bool enableQrn{};
    bool enableQsb{};
    bool enableFlutter{};
    int sampleRate{11025};
    float centerPitchHz{600.0f};
    float bandwidthHz{500.0f};
};

class RunnerAudioEffects
{
public:
    static std::vector<float> renderMorseText(const std::string& text, int speedCpm, float pitch, SType signalType, int sampleRate = 44100);
    static std::vector<float> renderIncomingSignal(
        const std::string& text,
        int speedCpm,
        const RunnerBandEffectsConfig& config,
        const std::vector<std::string>& qrmTexts,
        SType signalType);
    static void applyBackgroundStatic(std::vector<float>& buffer);
    static std::vector<float> createStationBackground(int sampleRate, double seconds, bool enableQrn, bool enableFlutter);
    static void applyQsb(std::vector<float>& buffer, int sampleRate);
    static void applyQrn(std::vector<float>& buffer);
    static void applyFlutter(std::vector<float>& buffer, int sampleRate);
    static void applyReceiverBandwidth(std::vector<float>& buffer, int sampleRate, float centerPitchHz, float bandwidthHz);
    static void applyAgc(std::vector<float>& buffer, int sampleRate);
    static void mixQrm(std::vector<float>& buffer, const std::vector<float>& interference, std::size_t offsetSamples = 0);
    static std::vector<float> applyIncomingEffects(
        std::vector<float> baseBuffer,
        const RunnerBandEffectsConfig& config,
        const std::vector<std::string>& qrmTexts,
        int qrmSpeedCpm,
        float qrmPitch,
        SType qrmSignalType);
};
