#include <catch2/catch_test_macros.hpp>

#include "AudioOutputDevice.h"
#include "StringGen.h"
#include "User.h"

#include <algorithm>
#include <stdexcept>
#include <vector>

TEST_CASE("Letters mode generates only letters")
{
    StringGen generator;
    const std::string result = generator.generate(64, GroupCharsetMode::Letters);
    const std::string& allowed = StringGen::charsetForMode(GroupCharsetMode::Letters);

    REQUIRE(result.size() == 64);
    REQUIRE(std::all_of(result.begin(), result.end(), [&allowed](char c) {
        return allowed.find(c) != std::string::npos;
    }));
}

TEST_CASE("Digits mode generates only digits")
{
    StringGen generator;
    const std::string result = generator.generate(64, GroupCharsetMode::Digits);
    const std::string& allowed = StringGen::charsetForMode(GroupCharsetMode::Digits);

    REQUIRE(result.size() == 64);
    REQUIRE(std::all_of(result.begin(), result.end(), [&allowed](char c) {
        return allowed.find(c) != std::string::npos;
    }));
}

TEST_CASE("Mixed profiles only use allowed characters")
{
    StringGen generator;

    SECTION("letters and digits")
    {
        const std::string result = generator.generate(128, GroupCharsetMode::LettersDigits);
        const std::string& allowed = StringGen::charsetForMode(GroupCharsetMode::LettersDigits);
        REQUIRE(std::all_of(result.begin(), result.end(), [&allowed](char c) {
            return allowed.find(c) != std::string::npos;
        }));
    }

    SECTION("letters digits and punctuation")
    {
        const std::string result = generator.generate(128, GroupCharsetMode::LettersDigitsPunctuation);
        const std::string& allowed = StringGen::charsetForMode(GroupCharsetMode::LettersDigitsPunctuation);
        REQUIRE(std::all_of(result.begin(), result.end(), [&allowed](char c) {
            return allowed.find(c) != std::string::npos;
        }));
    }
}

TEST_CASE("RUFZ default profile matches current mixed charset")
{
    const std::string& allowed = StringGen::charsetForMode(GroupCharsetMode::LettersDigitsPunctuation);

    REQUIRE(allowed.find('a') != std::string::npos);
    REQUIRE(allowed.find('1') != std::string::npos);
    REQUIRE(allowed.find('.') != std::string::npos);
    REQUIRE(allowed.find('?') != std::string::npos);
}

TEST_CASE("Custom trainer charset generation only uses selected symbols")
{
    StringGen generator;
    const std::string result = generator.generateFromCharset(64, "ab19");

    REQUIRE(result.size() == 64);
    REQUIRE(std::all_of(result.begin(), result.end(), [](char c) {
        return c == 'a' || c == 'b' || c == '1' || c == '9';
    }));
}

TEST_CASE("Trainer settings default to useful trainer values")
{
    TrainerSettings settings;

    REQUIRE(settings.fixedSpeed);
    REQUIRE(settings.speed == 20);
    REQUIRE(settings.groupLength == 5);
    REQUIRE_FALSE(settings.allowedSymbols.empty());
}

TEST_CASE("Audio output device resolution validates selected indices")
{
    const std::vector<AudioOutputDevice> devices{
        {0, "Primary"},
        {1, "Secondary"}
    };

    const AudioOutputDevice resolved = AudioOutputDeviceService::resolvePlaybackDevice(devices, 1);
    REQUIRE(resolved.index == 1);
    REQUIRE(resolved.name == "Secondary");

    REQUIRE_THROWS_AS(AudioOutputDeviceService::resolvePlaybackDevice(devices, 99), std::out_of_range);
}

TEST_CASE("Audio device enumeration is callable")
{
    REQUIRE_NOTHROW(AudioOutputDeviceService::listPlaybackDevices());
}
