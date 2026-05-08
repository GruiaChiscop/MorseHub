#pragma once

#include <string>

#include "SignalGenerator.h"

enum class RunnerResultKind
{
    None,
    Copied,
    AfterRepeat,
    Agn,
    B4,
    Bust
};

struct Result
{
    int id{};
    std::string text;
    std::string typedText;
    std::string runnerEnteredCall;
    std::string runnerEnteredRst;
    std::string runnerEnteredNr;
    std::string runnerSentText;
    int possiblePoints{};
    int points{};
    int elapsedMilliseconds{};
    int speed{};
    int frequency{};
    SType signalType{Sine};
    bool usedRepeat{};
    int errorCount{};
    RunnerResultKind runnerResultKind{RunnerResultKind::None};
    std::string runnerResponseText;
    std::string runnerRetransmittedText;
    std::string runnerStateText;
};
