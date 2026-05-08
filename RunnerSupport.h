#pragma once

#include <string>
#include <vector>

#include "Result.h"
#include "RunnerStation.h"

enum class RunnerCallMatch
{
    No,
    Almost,
    Exact
};

enum class RunnerDxState
{
    NeedPrevEnd,
    NeedQso,
    NeedNr,
    NeedCall,
    NeedCorrectedCall,
    NeedEnd,
    Done,
    Failed
};

struct RunnerOperatorInput
{
    std::string call;
    std::string rst;
    std::string nr;
};

struct RunnerAttemptResolution
{
    RunnerDxState nextState{RunnerDxState::NeedCall};
    RunnerResultKind resultKind{RunnerResultKind::None};
    bool shouldRepeat{};
    bool isComplete{};
    bool isFailure{};
};

struct RunnerQsoStep
{
    RunnerDxState previousState{RunnerDxState::NeedQso};
    RunnerDxState nextState{RunnerDxState::NeedQso};
    RunnerResultKind resultKind{RunnerResultKind::None};
    std::string sentText;
    std::string dxReplyText;
    std::string retransmittedText;
    bool isRoundComplete{};
    bool isRoundFailure{};
    bool isSuccessfulQso{};
};

class RunnerSupport
{
public:
    static RunnerStation generateSingleCallStation(int speedCpm);
    static RunnerRound generatePileupRound(int baseSpeedCpm, int pitch, SType signalType);
    static std::string renderPileupText(const std::vector<RunnerStation>& stations);
    static std::string formatExchangeText(const std::string& myCall, int serialNumber, bool includeDe = true);
    static std::string formatDxReplyText(const RunnerStation& station);
    static std::string extractWpxPrefix(const std::string& callsign);
    static std::string stateLabel(RunnerDxState state);
    static RunnerCallMatch classifyCallCopy(const std::string& expected, const std::string& typed);
    static RunnerCallMatch applyLidsBehavior(RunnerCallMatch match, const std::string& typed, bool lidsEnabled, double roll);
    static RunnerQsoStep processOperatorInput(
        RunnerDxState state,
        const RunnerStation& station,
        const std::string& myCall,
        int qsoSerialNumber,
        const RunnerOperatorInput& input,
        bool isPileupMode,
        bool lidsEnabled,
        double lidsRoll);
    static RunnerAttemptResolution resolveCallAttempt(RunnerDxState state, RunnerCallMatch match, bool usedRepeat);
    static bool isPrefixMatch(const std::string& expected, const std::string& typed);
    static bool isExactMatch(const std::string& expected, const std::string& typed);
    static std::string normalize(const std::string& text);
};
