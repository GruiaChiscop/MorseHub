#include <catch2/catch_test_macros.hpp>

#include "RufzScoring.h"

TEST_CASE("Exact match awards full points and increases speed")
{
    RufzScoringEngine engine({120, 10, 50, true, 0.5, 20, 2.0, 0.95, 1.05});
    RufzSessionState state{0, 0, 120, 0, 0};
    RufzRoundInput input{"K1ABC", "k1abc", 120, 5000, false};

    const RufzRoundResult result = engine.evaluate(state, input);

    REQUIRE(result.exactMatch);
    REQUIRE(result.errorCount == 0);
    REQUIRE(result.awardedPoints == 2 * 5 * 120);
    REQUIRE(result.nextSpeedCpm == 130);
}

TEST_CASE("One to three errors produce partial score")
{
    RufzScoringEngine engine({100, 10, 50, true, 0.5, 20, 2.0, 0.95, 1.05});
    RufzSessionState state{0, 0, 100, 0, 0};

    SECTION("one error")
    {
        const RufzRoundResult result = engine.evaluate(state, {"K1ABC", "K1ABD", 100, 6000, false});
        REQUIRE(result.partialMatch);
        REQUIRE(result.errorCount == 1);
        REQUIRE(result.awardedPoints == (2 * 5 * 100) / 5);
    }

    SECTION("two errors")
    {
        const RufzRoundResult result = engine.evaluate(state, {"K1ABC", "K1XYZ", 100, 6000, false});
        REQUIRE(result.partialMatch);
        REQUIRE(result.errorCount == 3);
        REQUIRE(result.awardedPoints == (2 * 5 * 100) / 15);
    }
}

TEST_CASE("Four or more errors produce zero points")
{
    RufzScoringEngine engine({90, 10, 50, true, 0.5, 20, 2.0, 0.95, 1.05});
    RufzSessionState state{0, 0, 90, 0, 0};

    const RufzRoundResult result = engine.evaluate(state, {"K1ABC", "ZZZZZ", 90, 1100, false});

    REQUIRE_FALSE(result.exactMatch);
    REQUIRE_FALSE(result.partialMatch);
    REQUIRE(result.errorCount >= 4);
    REQUIRE(result.awardedPoints == 0);
    REQUIRE(result.nextSpeedCpm == 80);
}

TEST_CASE("Repeat applies a fifty percent penalty after scoring")
{
    RufzScoringEngine engine({100, 10, 50, true, 0.5, 20, 2.0, 0.95, 1.05});
    RufzSessionState state{0, 0, 100, 0, 0};

    const RufzRoundResult result = engine.evaluate(state, {"K1ABC", "K1ABC", 100, 6000, true});

    REQUIRE(result.exactMatch);
    REQUIRE(result.awardedPoints == (2 * 5 * 100) / 2);
}

TEST_CASE("Speed never drops below configured minimum")
{
    RufzScoringEngine engine({25, 10, 50, true, 0.5, 20, 2.0, 0.95, 1.05});
    RufzSessionState state{0, 0, 25, 0, 0};

    const RufzRoundResult result = engine.evaluate(state, {"K1ABC", "WRONG", 25, 900, false});

    REQUIRE(result.nextSpeedCpm == 20);
}

TEST_CASE("Applying round results accumulates session state")
{
    RufzScoringEngine engine({100, 10, 50, true, 0.5, 20, 2.0, 0.95, 1.05});
    RufzSessionState state{0, 0, 100, 0, 0};

    const RufzRoundResult first = engine.evaluate(state, {"K1ABC", "K1ABC", 100, 6000, false});
    engine.applyResult(state, first);
    REQUIRE(state.roundIndex == 1);
    REQUIRE(state.totalPoints == first.awardedPoints);
    REQUIRE(state.currentSpeedCpm == 110);
    REQUIRE(state.consecutiveCorrect == 1);
    REQUIRE(state.totalErrors == 0);

    const RufzRoundResult second = engine.evaluate(state, {"K1ABC", "K1ABX", state.currentSpeedCpm, 6000, false});
    engine.applyResult(state, second);
    REQUIRE(state.roundIndex == 2);
    REQUIRE(state.totalPoints == first.awardedPoints + second.awardedPoints);
    REQUIRE(state.currentSpeedCpm == 100);
    REQUIRE(state.consecutiveCorrect == 0);
    REQUIRE(state.totalErrors == 1);
}

TEST_CASE("Faster exact copies award more points than slower exact copies")
{
    RufzScoringEngine engine({100, 10, 50, true, 0.5, 20, 2.0, 0.95, 1.05});
    RufzSessionState state{0, 0, 100, 0, 0};

    const RufzRoundResult fast = engine.evaluate(state, {"K1ABC", "K1ABC", 100, 2500, false});
    const RufzRoundResult slow = engine.evaluate(state, {"K1ABC", "K1ABC", 100, 12000, false});

    REQUIRE(fast.awardedPoints > slow.awardedPoints);
    REQUIRE(fast.awardedPoints <= 2 * 5 * 100 * 1.05);
    REQUIRE(slow.awardedPoints >= 2 * 5 * 100 * 0.95);
}

TEST_CASE("Consecutive exact copies do not change score by themselves")
{
    RufzScoringEngine engine({100, 10, 50, true, 0.5, 20, 2.0, 0.95, 1.05});
    RufzSessionState freshState{0, 0, 100, 0, 0};
    RufzSessionState streakState{2, 0, 100, 2, 0};

    const RufzRoundResult baseline = engine.evaluate(freshState, {"K1ABC", "K1ABC", 100, 6000, false});
    const RufzRoundResult streaked = engine.evaluate(streakState, {"K1ABC", "K1ABC", 100, 6000, false});

    REQUIRE(streaked.awardedPoints == baseline.awardedPoints);
}
