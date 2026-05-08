#include <catch2/catch_test_macros.hpp>

#include "RunnerSupport.h"

TEST_CASE("Pile-up rounds generate multiple callers and target the last one")
{
    const RunnerRound round = RunnerSupport::generatePileupRound(120, 550, Sine);

    REQUIRE(round.kind == RunnerRoundKind::Pileup);
    REQUIRE(round.stations.size() >= 2);
    REQUIRE(round.stations.size() <= 4);
    REQUIRE(round.targetIndex == round.stations.size() - 1);
    REQUIRE(round.playbackSpeedCpm == round.stations.back().speedCpm);
    REQUIRE(round.pitch == 550);
    REQUIRE(round.signalType == Sine);
    REQUIRE_FALSE(round.transmittedText.empty());
}

TEST_CASE("Pile-up rendering keeps callers in order")
{
    const std::vector<RunnerStation> stations{
        {"YO1ABC", 120},
        {"DL2XYZ", 130},
        {"K1TEST", 125}
    };

    REQUIRE(RunnerSupport::renderPileupText(stations) == "YO1ABC DL2XYZ K1TEST");
}

TEST_CASE("Prefix matches only accept shorter leading copies")
{
    REQUIRE(RunnerSupport::isPrefixMatch("YO1ABC", "YO1"));
    REQUIRE_FALSE(RunnerSupport::isPrefixMatch("YO1ABC", ""));
    REQUIRE_FALSE(RunnerSupport::isPrefixMatch("YO1ABC", "YO1ABC"));
    REQUIRE_FALSE(RunnerSupport::isPrefixMatch("YO1ABC", "DL2"));
}

TEST_CASE("Runner call classifier distinguishes exact almost and no")
{
    REQUIRE(RunnerSupport::classifyCallCopy("YO1ABC", "YO1ABC") == RunnerCallMatch::Exact);
    REQUIRE(RunnerSupport::classifyCallCopy("YO1ABC", "YO1A") == RunnerCallMatch::Almost);
    REQUIRE(RunnerSupport::classifyCallCopy("YO1ABC", "YO1A?C") == RunnerCallMatch::Almost);
    REQUIRE(RunnerSupport::classifyCallCopy("YO1ABC", "YO2ABC") == RunnerCallMatch::Almost);
    REQUIRE(RunnerSupport::classifyCallCopy("YO1ABC", "Y") == RunnerCallMatch::No);
    REQUIRE(RunnerSupport::classifyCallCopy("YO1ABC", "DL2XYZ") == RunnerCallMatch::No);
}

TEST_CASE("LIDS behavior follows Morse Runner style thresholds")
{
    SECTION("two-character partial copies are rejected when LIDS is off")
    {
        REQUIRE(RunnerSupport::applyLidsBehavior(RunnerCallMatch::Almost, "YO", false, 0.5) == RunnerCallMatch::No);
    }

    SECTION("LIDS can occasionally downgrade an exact copy")
    {
        REQUIRE(RunnerSupport::applyLidsBehavior(RunnerCallMatch::Exact, "YO1ABC", true, 0.0) == RunnerCallMatch::Almost);
    }

    SECTION("LIDS can occasionally accept an almost copy")
    {
        REQUIRE(RunnerSupport::applyLidsBehavior(RunnerCallMatch::Almost, "YO1AB", true, 0.0) == RunnerCallMatch::Exact);
    }

    SECTION("without LIDS the classifier result stays unchanged for longer copies")
    {
        REQUIRE(RunnerSupport::applyLidsBehavior(RunnerCallMatch::Almost, "YO1AB", false, 0.0) == RunnerCallMatch::Almost);
    }
}

TEST_CASE("Runner attempt resolution follows need call and corrected call states")
{
    SECTION("almost on first try requests AGN and repeat")
    {
        const RunnerAttemptResolution resolution = RunnerSupport::resolveCallAttempt(RunnerDxState::NeedCall, RunnerCallMatch::Almost, false);
        REQUIRE(resolution.nextState == RunnerDxState::NeedCorrectedCall);
        REQUIRE(resolution.resultKind == RunnerResultKind::Agn);
        REQUIRE(resolution.shouldRepeat);
        REQUIRE_FALSE(resolution.isComplete);
        REQUIRE_FALSE(resolution.isFailure);
    }

    SECTION("exact after repeat completes with penalty marker")
    {
        const RunnerAttemptResolution resolution = RunnerSupport::resolveCallAttempt(RunnerDxState::NeedCorrectedCall, RunnerCallMatch::Exact, true);
        REQUIRE(resolution.nextState == RunnerDxState::Done);
        REQUIRE(resolution.resultKind == RunnerResultKind::AfterRepeat);
        REQUIRE_FALSE(resolution.shouldRepeat);
        REQUIRE(resolution.isComplete);
        REQUIRE_FALSE(resolution.isFailure);
    }

    SECTION("almost on corrected call becomes B4")
    {
        const RunnerAttemptResolution resolution = RunnerSupport::resolveCallAttempt(RunnerDxState::NeedCorrectedCall, RunnerCallMatch::Almost, true);
        REQUIRE(resolution.nextState == RunnerDxState::Failed);
        REQUIRE(resolution.resultKind == RunnerResultKind::B4);
        REQUIRE_FALSE(resolution.shouldRepeat);
        REQUIRE_FALSE(resolution.isComplete);
        REQUIRE(resolution.isFailure);
    }

    SECTION("no match busts immediately")
    {
        const RunnerAttemptResolution resolution = RunnerSupport::resolveCallAttempt(RunnerDxState::NeedCall, RunnerCallMatch::No, false);
        REQUIRE(resolution.nextState == RunnerDxState::Failed);
        REQUIRE(resolution.resultKind == RunnerResultKind::Bust);
        REQUIRE_FALSE(resolution.shouldRepeat);
        REQUIRE_FALSE(resolution.isComplete);
        REQUIRE(resolution.isFailure);
    }
}

TEST_CASE("Runner QSO flow follows original-style call exchange and TU states")
{
    RunnerStation station{"YO1ABC", 120, 599, 123};

    SECTION("exact call sends exchange and waits for TU")
    {
        const RunnerQsoStep step = RunnerSupport::processOperatorInput(
            RunnerDxState::NeedQso,
            station,
            "YO9TEST",
            7,
            {"YO1ABC", "", ""},
            true,
            false,
            0.5);

        REQUIRE(step.resultKind == RunnerResultKind::Copied);
        REQUIRE(step.sentText == "DE YO9TEST 5NN007");
        REQUIRE(step.dxReplyText == "R 5NN123");
        REQUIRE(step.nextState == RunnerDxState::NeedEnd);
        REQUIRE_FALSE(step.isRoundComplete);
    }

    SECTION("almost call asks for correction and repeats the target")
    {
        const RunnerQsoStep step = RunnerSupport::processOperatorInput(
            RunnerDxState::NeedQso,
            station,
            "YO9TEST",
            7,
            {"YO1AB", "", ""},
            true,
            false,
            0.5);

        REQUIRE(step.resultKind == RunnerResultKind::Agn);
        REQUIRE(step.sentText == "DE YO9TEST");
        REQUIRE(step.dxReplyText == "AGN");
        REQUIRE(step.retransmittedText == "YO1ABC");
        REQUIRE(step.nextState == RunnerDxState::NeedCorrectedCall);
    }

    SECTION("corrected call plus number resumes QSO")
    {
        const RunnerQsoStep step = RunnerSupport::processOperatorInput(
            RunnerDxState::NeedCorrectedCall,
            station,
            "YO9TEST",
            7,
            {"YO1ABC", "599", "007"},
            true,
            false,
            0.5);

        REQUIRE(step.resultKind == RunnerResultKind::AfterRepeat);
        REQUIRE(step.sentText == "DE YO9TEST 5NN007");
        REQUIRE(step.dxReplyText == "R 5NN123");
        REQUIRE(step.nextState == RunnerDxState::NeedEnd);
    }

    SECTION("empty enter on NeedEnd sends TU and completes")
    {
        const RunnerQsoStep step = RunnerSupport::processOperatorInput(
            RunnerDxState::NeedEnd,
            station,
            "YO9TEST",
            7,
            {"", "", ""},
            true,
            false,
            0.5);

        REQUIRE(step.resultKind == RunnerResultKind::Copied);
        REQUIRE(step.sentText == "TU YO9TEST");
        REQUIRE(step.nextState == RunnerDxState::Done);
        REQUIRE(step.isRoundComplete);
        REQUIRE(step.isSuccessfulQso);
    }
}

TEST_CASE("WPX prefix extraction keeps the leading prefix through the first digit")
{
    REQUIRE(RunnerSupport::extractWpxPrefix("YO1ABC") == "YO1");
    REQUIRE(RunnerSupport::extractWpxPrefix("DL2XYZ") == "DL2");
    REQUIRE(RunnerSupport::extractWpxPrefix("K1TEST") == "K1");
    REQUIRE(RunnerSupport::extractWpxPrefix("ABC") == "ABC");
}
