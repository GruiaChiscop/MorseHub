#include "RunnerSupport.h"

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <sstream>
#include <vector>
#include <random>

#include "StringGen.h"

namespace
{
std::mt19937& rng()
{
    static std::mt19937 generator(std::random_device{}());
    return generator;
}

std::size_t visibleCharacterCount(const std::string& text)
{
    return static_cast<std::size_t>(std::count_if(text.begin(), text.end(), [](char c) {
        return c != '?';
    }));
}

std::string formatSerialNumber(int serialNumber)
{
    std::ostringstream stream;
    stream << std::setfill('0') << std::setw(3) << std::max(0, serialNumber);
    return stream.str();
}
}

RunnerStation RunnerSupport::generateSingleCallStation(int speedCpm)
{
    StringGen generator;
    RunnerStation station;
    station.speedCpm = speedCpm;
    station.callsign = normalize(generator.generateLettersAndDigits(5));
    std::uniform_int_distribution<int> serialDistribution(1, 999);
    station.serialNumber = serialDistribution(rng());
    return station;
}

RunnerRound RunnerSupport::generatePileupRound(int baseSpeedCpm, int pitch, SType signalType)
{
    std::uniform_int_distribution<int> countDistribution(2, 4);
    std::uniform_int_distribution<int> speedOffsetDistribution(-10, 15);

    RunnerRound round;
    round.kind = RunnerRoundKind::Pileup;
    round.targetIndex = 0;
    round.pitch = pitch;
    round.signalType = signalType;

    const int count = countDistribution(rng());
    round.stations.reserve(static_cast<std::size_t>(count));
    for (int index = 0; index < count; ++index)
    {
        const int stationSpeed = std::max(40, baseSpeedCpm + speedOffsetDistribution(rng()));
        round.stations.push_back(generateSingleCallStation(stationSpeed));
    }

    round.targetIndex = round.stations.empty() ? 0 : round.stations.size() - 1;
    round.playbackSpeedCpm = round.stations.empty() ? baseSpeedCpm : round.stations[round.targetIndex].speedCpm;
    round.transmittedText = renderPileupText(round.stations);
    return round;
}

std::string RunnerSupport::renderPileupText(const std::vector<RunnerStation>& stations)
{
    std::string rendered;
    for (std::size_t index = 0; index < stations.size(); ++index)
    {
        if (!rendered.empty())
            rendered += ' ';
        rendered += normalize(stations[index].callsign);
    }
    return rendered;
}

std::string RunnerSupport::formatExchangeText(const std::string& myCall, int serialNumber, bool includeDe)
{
    const std::string exchange = "5NN" + formatSerialNumber(serialNumber);
    if (includeDe)
        return "DE " + normalize(myCall) + " " + exchange;
    return normalize(myCall) + " " + exchange;
}

std::string RunnerSupport::formatDxReplyText(const RunnerStation& station)
{
    return "R 5NN" + formatSerialNumber(station.serialNumber);
}

std::string RunnerSupport::extractWpxPrefix(const std::string& callsign)
{
    const std::string normalized = normalize(callsign);
    if (normalized.empty())
        return {};

    for (std::size_t i = 0; i < normalized.size(); ++i)
    {
        if (std::isdigit(static_cast<unsigned char>(normalized[i])))
            return normalized.substr(0, i + 1);
    }

    return normalized.substr(0, std::min<std::size_t>(3, normalized.size()));
}

std::string RunnerSupport::stateLabel(RunnerDxState state)
{
    switch (state)
    {
    case RunnerDxState::NeedPrevEnd:
        return "Need previous QSO end";
    case RunnerDxState::NeedQso:
        return "Need call";
    case RunnerDxState::NeedNr:
        return "Need exchange";
    case RunnerDxState::NeedCall:
        return "Need corrected call";
    case RunnerDxState::NeedCorrectedCall:
        return "Need corrected call and exchange";
    case RunnerDxState::NeedEnd:
        return "Need TU";
    case RunnerDxState::Done:
        return "Done";
    case RunnerDxState::Failed:
    default:
        return "Failed";
    }
}

RunnerCallMatch RunnerSupport::classifyCallCopy(const std::string& expected, const std::string& typed)
{
    const std::string normalizedExpected = normalize(expected);
    const std::string normalizedTyped = normalize(typed);

    if (normalizedExpected.empty() || normalizedTyped.empty())
        return RunnerCallMatch::No;

    if (normalizedExpected == normalizedTyped)
        return RunnerCallMatch::Exact;

    const std::size_t visibleTypedCharacters = visibleCharacterCount(normalizedTyped);
    if (visibleTypedCharacters < 2)
        return RunnerCallMatch::No;

    std::vector<std::vector<int>> penalties(
        normalizedTyped.size() + 1,
        std::vector<int>(normalizedExpected.size() + 1, 0));

    for (std::size_t col = 1; col <= normalizedExpected.size(); ++col)
        penalties[0][col] = 0;
    for (std::size_t row = 1; row <= normalizedTyped.size(); ++row)
        penalties[row][0] = penalties[row - 1][0] + 2;

    for (std::size_t row = 1; row <= normalizedTyped.size(); ++row)
    {
        for (std::size_t col = 1; col <= normalizedExpected.size(); ++col)
        {
            int top = penalties[row][col - 1];
            if (row < normalizedTyped.size() && normalizedTyped[row - 1] != '?')
                top += 2;

            int left = penalties[row - 1][col];
            if (normalizedTyped[row - 1] != '?')
                left += 2;

            int diagonal = penalties[row - 1][col - 1];
            if (!(normalizedTyped[row - 1] == normalizedExpected[col - 1] || normalizedTyped[row - 1] == '?'))
                diagonal += 2;

            penalties[row][col] = std::min({top, left, diagonal});
        }
    }

    const int totalPenalty = penalties[normalizedTyped.size()][normalizedExpected.size()];
    if (totalPenalty == 0)
        return RunnerCallMatch::Almost;
    if (totalPenalty <= 2)
        return RunnerCallMatch::Almost;
    return RunnerCallMatch::No;
}

RunnerCallMatch RunnerSupport::applyLidsBehavior(RunnerCallMatch match, const std::string& typed, bool lidsEnabled, double roll)
{
    const std::string normalizedTyped = normalize(typed);
    const std::size_t visibleTypedCharacters = visibleCharacterCount(normalizedTyped);

    if (!lidsEnabled && visibleTypedCharacters == 2 && match == RunnerCallMatch::Almost)
        return RunnerCallMatch::No;

    if (!lidsEnabled || normalizedTyped.size() <= 3)
        return match;

    if (match == RunnerCallMatch::Exact && roll < 0.01)
        return RunnerCallMatch::Almost;

    if (match == RunnerCallMatch::Almost && roll < 0.04)
        return RunnerCallMatch::Exact;

    return match;
}

RunnerQsoStep RunnerSupport::processOperatorInput(
    RunnerDxState state,
    const RunnerStation& station,
    const std::string& myCall,
    int qsoSerialNumber,
    const RunnerOperatorInput& input,
    bool isPileupMode,
    bool lidsEnabled,
    double lidsRoll)
{
    const std::string normalizedCall = normalize(input.call);
    const std::string normalizedNr = normalize(input.nr);
    const bool hasCall = !normalizedCall.empty();
    const bool hasNumber = !normalizedNr.empty();
    RunnerCallMatch match = hasCall ? classifyCallCopy(station.callsign, normalizedCall) : RunnerCallMatch::No;
    match = applyLidsBehavior(match, normalizedCall, lidsEnabled, lidsRoll);

    RunnerQsoStep step;
    step.previousState = state;
    step.nextState = state;

    auto completeSuccess = [&](RunnerResultKind kind, const std::string& sentText) {
        step.resultKind = kind;
        step.sentText = sentText;
        step.nextState = RunnerDxState::Done;
        step.isRoundComplete = true;
        step.isSuccessfulQso = true;
    };

    switch (state)
    {
    case RunnerDxState::NeedPrevEnd:
    case RunnerDxState::NeedQso:
        if (match == RunnerCallMatch::Exact)
        {
            step.resultKind = RunnerResultKind::Copied;
            step.sentText = formatExchangeText(myCall, qsoSerialNumber);
            step.dxReplyText = formatDxReplyText(station);
            step.nextState = RunnerDxState::NeedEnd;
        }
        else if (match == RunnerCallMatch::Almost)
        {
            step.resultKind = RunnerResultKind::Agn;
            step.sentText = "DE " + normalize(myCall);
            step.dxReplyText = "AGN";
            step.retransmittedText = station.callsign;
            step.nextState = RunnerDxState::NeedCorrectedCall;
        }
        else
        {
            step.resultKind = RunnerResultKind::Bust;
            step.sentText = "?";
            step.dxReplyText = "NIL";
            step.nextState = RunnerDxState::Failed;
            step.isRoundComplete = true;
            step.isRoundFailure = true;
        }
        break;

    case RunnerDxState::NeedNr:
        if (hasNumber)
        {
            step.resultKind = RunnerResultKind::Copied;
            step.sentText = formatExchangeText(myCall, qsoSerialNumber, false);
            step.dxReplyText = formatDxReplyText(station);
            step.nextState = RunnerDxState::NeedEnd;
        }
        else
        {
            step.resultKind = RunnerResultKind::Agn;
            step.sentText = "NR?";
            step.dxReplyText = "AGN";
            step.nextState = RunnerDxState::NeedNr;
        }
        break;

    case RunnerDxState::NeedCall:
        if (match == RunnerCallMatch::Exact)
        {
            step.resultKind = RunnerResultKind::Copied;
            step.sentText = formatExchangeText(myCall, qsoSerialNumber);
            step.dxReplyText = formatDxReplyText(station);
            step.nextState = RunnerDxState::NeedEnd;
        }
        else if (match == RunnerCallMatch::Almost)
        {
            step.resultKind = RunnerResultKind::B4;
            step.sentText = "QSO B4";
            step.dxReplyText = "DE " + normalize(myCall);
            step.retransmittedText = station.callsign;
            step.nextState = RunnerDxState::NeedCall;
        }
        else
        {
            step.resultKind = RunnerResultKind::Bust;
            step.sentText = "?";
            step.dxReplyText = "NIL";
            step.nextState = RunnerDxState::Failed;
            step.isRoundComplete = true;
            step.isRoundFailure = true;
        }
        break;

    case RunnerDxState::NeedCorrectedCall:
        if (match == RunnerCallMatch::Exact && hasNumber)
        {
            step.resultKind = RunnerResultKind::AfterRepeat;
            step.sentText = formatExchangeText(myCall, qsoSerialNumber);
            step.dxReplyText = formatDxReplyText(station);
            step.nextState = RunnerDxState::NeedEnd;
        }
        else if (match == RunnerCallMatch::Exact)
        {
            step.resultKind = RunnerResultKind::Agn;
            step.sentText = "NR?";
            step.dxReplyText = "AGN";
            step.nextState = RunnerDxState::NeedNr;
        }
        else if (match == RunnerCallMatch::Almost)
        {
            step.resultKind = RunnerResultKind::B4;
            step.sentText = "QSO B4";
            step.dxReplyText = "DE " + normalize(myCall);
            step.retransmittedText = station.callsign;
            step.nextState = RunnerDxState::NeedCorrectedCall;
        }
        else
        {
            step.resultKind = RunnerResultKind::Bust;
            step.sentText = "?";
            step.dxReplyText = "NIL";
            step.nextState = RunnerDxState::Failed;
            step.isRoundComplete = true;
            step.isRoundFailure = true;
        }
        break;

    case RunnerDxState::NeedEnd:
        if (!hasCall && !hasNumber)
        {
            completeSuccess(
                step.resultKind == RunnerResultKind::AfterRepeat ? RunnerResultKind::AfterRepeat : RunnerResultKind::Copied,
                isPileupMode ? "TU " + normalize(myCall) : "TU");
        }
        else
        {
            step.resultKind = RunnerResultKind::B4;
            step.sentText = "QSO B4";
            step.dxReplyText = "DE " + normalize(myCall);
            step.nextState = RunnerDxState::NeedCall;
        }
        break;

    case RunnerDxState::Done:
        completeSuccess(RunnerResultKind::Copied, isPileupMode ? "TU " + normalize(myCall) : "TU");
        break;

    case RunnerDxState::Failed:
        step.resultKind = RunnerResultKind::Bust;
        step.nextState = RunnerDxState::Failed;
        step.isRoundComplete = true;
        step.isRoundFailure = true;
        break;
    }

    return step;
}

RunnerAttemptResolution RunnerSupport::resolveCallAttempt(RunnerDxState state, RunnerCallMatch match, bool usedRepeat)
{
    if (match == RunnerCallMatch::Exact)
        return {RunnerDxState::Done, usedRepeat ? RunnerResultKind::AfterRepeat : RunnerResultKind::Copied, false, true, false};

    if (state == RunnerDxState::NeedCall && match == RunnerCallMatch::Almost)
        return {RunnerDxState::NeedCorrectedCall, RunnerResultKind::Agn, true, false, false};

    if (state == RunnerDxState::NeedCorrectedCall && match == RunnerCallMatch::Almost)
        return {RunnerDxState::Failed, RunnerResultKind::B4, false, false, true};

    return {RunnerDxState::Failed, RunnerResultKind::Bust, false, false, true};
}

bool RunnerSupport::isPrefixMatch(const std::string& expected, const std::string& typed)
{
    const std::string normalizedExpected = normalize(expected);
    const std::string normalizedTyped = normalize(typed);
    if (normalizedTyped.empty() || normalizedTyped.size() >= normalizedExpected.size())
        return false;
    return normalizedExpected.starts_with(normalizedTyped);
}

bool RunnerSupport::isExactMatch(const std::string& expected, const std::string& typed)
{
    return normalize(expected) == normalize(typed);
}

std::string RunnerSupport::normalize(const std::string& text)
{
    std::string normalized = text;
    std::transform(normalized.begin(), normalized.end(), normalized.begin(), [](unsigned char c) {
        return static_cast<char>(std::toupper(c));
    });
    return normalized;
}
