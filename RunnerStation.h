#pragma once

#include <string>
#include <vector>

#include "SignalGenerator.h"

enum class RunnerRoundKind
{
    SingleCall,
    Pileup
};

struct RunnerStation
{
    std::string callsign;
    int speedCpm{};
    int rst{599};
    int serialNumber{1};
};

struct RunnerRound
{
    RunnerRoundKind kind{RunnerRoundKind::SingleCall};
    std::vector<RunnerStation> stations;
    std::size_t targetIndex{};
    std::string transmittedText;
    int playbackSpeedCpm{};
    int pitch{};
    SType signalType{Sine};
};
