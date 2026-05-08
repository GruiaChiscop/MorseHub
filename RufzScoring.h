#pragma once

#include <string>

struct RufzSessionConfig
{
    int initialSpeedCpm{100};
    int speedStepCpm{10};
    int maxRounds{50};
    bool allowRepeat{true};
    double repeatPenalty{0.5};
    int minSpeedCpm{20};
    double targetCopyTimeFactor{2.0};
    double minTimeMultiplier{0.95};
    double maxTimeMultiplier{1.05};
};

struct RufzRoundInput
{
    std::string expected;
    std::string typed;
    int speedCpm{};
    int elapsedMs{};
    bool usedRepeat{};
};

struct RufzRoundResult
{
    int awardedPoints{};
    int nextSpeedCpm{};
    int errorCount{};
    bool exactMatch{};
    bool partialMatch{};
    std::string feedbackText;
};

struct RufzSessionState
{
    int roundIndex{};
    int totalPoints{};
    int currentSpeedCpm{};
    int consecutiveCorrect{};
    int totalErrors{};
};

class RufzScoringEngine
{
public:
    explicit RufzScoringEngine(RufzSessionConfig config = {});

    const RufzSessionConfig& config() const { return m_config; }

    RufzRoundResult evaluate(const RufzSessionState& state, const RufzRoundInput& input) const;
    void applyResult(RufzSessionState& state, const RufzRoundResult& result) const;

private:
    RufzSessionConfig m_config;

    static std::string normalize(const std::string& text);
    static int countErrors(const std::string& expected, const std::string& typed);
    double timeMultiplier(const RufzRoundInput& input, std::size_t textLength) const;
};
