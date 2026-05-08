#include "RufzScoring.h"

#include <algorithm>
#include <cctype>
#include <cmath>

namespace
{
int clampToMinimum(int value, int minimum)
{
    return std::max(value, minimum);
}

int truncatedPoints(double value)
{
    return static_cast<int>(value);
}
}

RufzScoringEngine::RufzScoringEngine(RufzSessionConfig config) : m_config(config)
{
}

RufzRoundResult RufzScoringEngine::evaluate(const RufzSessionState& state, const RufzRoundInput& input) const
{
    const std::string expected = normalize(input.expected);
    const std::string typed = normalize(input.typed);
    const int speed = input.speedCpm > 0 ? input.speedCpm : state.currentSpeedCpm;
    const double basePoints = static_cast<double>(2 * expected.size() * speed);
    const double timeScoreMultiplier = timeMultiplier(input, expected.size());

    RufzRoundResult result;
    result.nextSpeedCpm = clampToMinimum(speed, m_config.minSpeedCpm);
    result.errorCount = countErrors(expected, typed);
    result.exactMatch = (expected == typed);
    result.partialMatch = !result.exactMatch && result.errorCount > 0 && result.errorCount <= 3;

    if (result.exactMatch)
    {
        result.awardedPoints = truncatedPoints(basePoints * timeScoreMultiplier);
        result.nextSpeedCpm = speed + m_config.speedStepCpm;
        result.feedbackText = "Correct";
    }
    else
    {
        result.nextSpeedCpm = clampToMinimum(speed - m_config.speedStepCpm, m_config.minSpeedCpm);

        if (result.partialMatch)
        {
            result.awardedPoints = truncatedPoints((basePoints / (5.0 * result.errorCount)) * timeScoreMultiplier);
            result.feedbackText = "Partial copy";
        }
        else
        {
            result.awardedPoints = 0;
            result.feedbackText = "Incorrect";
        }
    }

    if (input.usedRepeat)
    {
        result.awardedPoints = truncatedPoints(static_cast<double>(result.awardedPoints) * m_config.repeatPenalty);
        result.feedbackText += " (repeat penalty)";
    }

    return result;
}

void RufzScoringEngine::applyResult(RufzSessionState& state, const RufzRoundResult& result) const
{
    state.roundIndex += 1;
    state.totalPoints += result.awardedPoints;
    state.currentSpeedCpm = result.nextSpeedCpm;

    if (result.exactMatch)
    {
        state.consecutiveCorrect += 1;
    }
    else
    {
        state.consecutiveCorrect = 0;
        state.totalErrors += result.errorCount > 0 ? result.errorCount : 1;
    }
}

std::string RufzScoringEngine::normalize(const std::string& text)
{
    std::string normalized = text;
    std::transform(normalized.begin(), normalized.end(), normalized.begin(), [](unsigned char c) {
        return static_cast<char>(std::toupper(c));
    });
    return normalized;
}

int RufzScoringEngine::countErrors(const std::string& expected, const std::string& typed)
{
    const std::size_t maxLength = std::max(expected.size(), typed.size());
    int errors = 0;

    for (std::size_t index = 0; index < maxLength; ++index)
    {
        const char expectedChar = index < expected.size() ? expected[index] : '\0';
        const char typedChar = index < typed.size() ? typed[index] : '\0';

        if (expectedChar != typedChar)
        {
            errors += 1;
        }
    }

    return errors;
}

double RufzScoringEngine::timeMultiplier(const RufzRoundInput& input, std::size_t textLength) const
{
    if (textLength == 0 || input.elapsedMs <= 0)
        return 1.0;

    const int speed = input.speedCpm > 0 ? input.speedCpm : m_config.initialSpeedCpm;
    const double referenceMsPerCharacter = 60000.0 / static_cast<double>(std::max(speed, 1));
    const double targetMs = std::max(250.0, referenceMsPerCharacter * static_cast<double>(textLength) * m_config.targetCopyTimeFactor);
    const double rawMultiplier = targetMs / static_cast<double>(input.elapsedMs);

    return std::clamp(rawMultiplier, m_config.minTimeMultiplier, m_config.maxTimeMultiplier);
}
