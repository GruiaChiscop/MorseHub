#pragma once

#include <string>
#include <vector>

struct ReceptionParsedEntry
{
    std::vector<std::string> groups;
    std::vector<std::size_t> invalidIndices;
};

struct ReceptionRowEvaluation
{
    int errorCount{};
    int correctGroupCount{};
    int incompleteGroupCount{};
    ReceptionParsedEntry parsed;
};

class ReceptionParser
{
public:
    static ReceptionParsedEntry parseGroups(const std::string& text, char separator);
    static ReceptionRowEvaluation evaluate(const std::vector<std::string>& expectedGroups, const std::string& enteredText, char separator);
    static ReceptionRowEvaluation evaluateGroups(const std::vector<std::string>& expectedGroups, const std::vector<std::string>& enteredGroups);
    static std::string normalizeForCell(const std::string& value);

private:
    static std::string trim(const std::string& value);
    static std::string normalizeGroup(const std::string& value);
    static int countGroupErrors(const std::string& expected, const std::string& actual);
};
