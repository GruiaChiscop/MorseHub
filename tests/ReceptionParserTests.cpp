#include <catch2/catch_test_macros.hpp>

#include "ReceptionParser.h"

TEST_CASE("Reception parser splits by separator and marks invalid groups")
{
    const ReceptionParsedEntry parsed = ReceptionParser::parseGroups("ABCDE;ABC;FGHIJ", ';');

    REQUIRE(parsed.groups.size() == 3);
    REQUIRE(parsed.groups[0] == "ABCDE");
    REQUIRE(parsed.groups[1] == "ABC");
    REQUIRE(parsed.groups[2] == "FGHIJ");
    REQUIRE(parsed.invalidIndices.size() == 1);
    REQUIRE(parsed.invalidIndices[0] == 1);
}

TEST_CASE("Reception parser falls back to whitespace splitting")
{
    const ReceptionParsedEntry parsed = ReceptionParser::parseGroups("ABCDE FGHIJ KLMNO", ';');

    REQUIRE(parsed.groups.size() == 3);
    REQUIRE(parsed.invalidIndices.empty());
}

TEST_CASE("Reception evaluation counts correct groups and character errors")
{
    const std::vector<std::string> expected{"ABCDE", "FGHIJ", "KLMNO"};
    const ReceptionRowEvaluation evaluation = ReceptionParser::evaluate(expected, "ABCDE;FGHIX;KLM", ';');

    REQUIRE(evaluation.correctGroupCount == 1);
    REQUIRE(evaluation.incompleteGroupCount == 1);
    REQUIRE(evaluation.errorCount == 3);
}

TEST_CASE("Reception evaluation from cells marks blank expected cells as incomplete")
{
    const std::vector<std::string> expected{"ABCDE", "FGHIJ", "KLMNO"};
    const std::vector<std::string> entered{"ABCDE", "", "KLMNO"};
    const ReceptionRowEvaluation evaluation = ReceptionParser::evaluateGroups(expected, entered);

    REQUIRE(evaluation.correctGroupCount == 2);
    REQUIRE(evaluation.incompleteGroupCount == 1);
    REQUIRE(evaluation.errorCount == 5);
}
