#include <catch2/catch_test_macros.hpp>

#include "RunnerAudioEffects.h"

TEST_CASE("Incoming runner effects preserve renderable audio buffers")
{
    RunnerBandEffectsConfig config;
    config.enableQsb = true;
    config.enableQrn = true;
    config.enableQrm = true;
    config.enableFlutter = true;
    config.sampleRate = 11025;
    config.centerPitchHz = 600.0f;
    config.bandwidthHz = 500.0f;

    std::vector<float> effected = RunnerAudioEffects::renderIncomingSignal(
        "CQ TEST",
        120,
        config,
        {"QRL", "CQ CQ TEST"},
        Sine);

    REQUIRE_FALSE(effected.empty());
    REQUIRE(effected.size() > static_cast<std::size_t>(config.sampleRate / 2));
}

TEST_CASE("Background static adds low-level noise to the received signal")
{
    std::vector<float> base(128, 0.0f);
    RunnerAudioEffects::applyBackgroundStatic(base);

    bool hasNonZeroSample = false;
    for (float sample : base)
    {
        if (sample != 0.0f)
        {
            hasNonZeroSample = true;
            break;
        }
    }

    REQUIRE(hasNonZeroSample);
}
