#include "StringGen.h"

#include <random>

namespace
{
const std::string kLetters = "abcdefghijklmnopqrstuvwxyz";
const std::string kDigits = "1234567890";
const std::string kLettersAndDigits = "abcdefghijklmnopqrstuvwxyz1234567890";
const std::string kLettersDigitsAndPunctuation = "abcdefghijklmnopqrstuvwxyz1234567890.,/?=+";

long long randomIndex(long long min, long long max)
{
    if (max <= min)
        return min;

    static std::mt19937_64 rng{std::random_device{}()};
    std::uniform_int_distribution<long long> distribution(min, max - 1);
    return distribution(rng);
}
}

std::string StringGen::generate(int length, GroupCharsetMode mode) const
{
    return generateFromCharset(length, charsetForMode(mode));
}

std::string StringGen::generateFromCharset(int length, const std::string& charset) const
{
    if (charset.empty())
        return {};

    std::string result;
    result.reserve(static_cast<std::size_t>(length));

    for (int i = 0; i < length; ++i)
    {
        const auto index = static_cast<std::size_t>(randomIndex(0, static_cast<long long>(charset.size())));
        result.push_back(charset[index]);
    }

    return result;
}

std::string StringGen::generateOnlyLetters(int length) const
{
    return generate(length, GroupCharsetMode::Letters);
}

std::string StringGen::generateOnlyDigits(int length) const
{
    return generate(length, GroupCharsetMode::Digits);
}

std::string StringGen::generateLettersAndDigits(int length) const
{
    return generate(length, GroupCharsetMode::LettersDigits);
}

const std::string& StringGen::charsetForMode(GroupCharsetMode mode)
{
    switch (mode)
    {
    case GroupCharsetMode::Letters:
        return kLetters;
    case GroupCharsetMode::Digits:
        return kDigits;
    case GroupCharsetMode::LettersDigits:
        return kLettersAndDigits;
    default:
        return kLettersDigitsAndPunctuation;
    }
}
