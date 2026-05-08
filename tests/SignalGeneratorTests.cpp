#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cmath>
#include <vector>

#include "SignalGenerator.h"

TEST_CASE("Sine wave uses a short attack and decay envelope")
{
    Generator generator(600.0f, 44100, Sine);
    std::vector<float> buffer;

    generator.addSineWave(0.05f, buffer);

    REQUIRE(buffer.size() > 10);
    REQUIRE(std::fabs(buffer.front()) < 0.001f);
    REQUIRE(std::fabs(buffer.back()) < 0.01f);
    REQUIRE(std::any_of(buffer.begin(), buffer.end(), [](float sample) {
        return std::fabs(sample) > 0.05f;
    }));
}

TEST_CASE("Silence keeps advancing without producing non-zero samples")
{
    Generator generator(600.0f, 44100, Sine);
    std::vector<float> buffer;

    generator.addSilence(0.02f, buffer);

    REQUIRE_FALSE(buffer.empty());
    REQUIRE(std::all_of(buffer.begin(), buffer.end(), [](float sample) {
        return sample == 0.0f;
    }));
}
