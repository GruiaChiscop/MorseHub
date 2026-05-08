#include "ReceptionParser.h"

#include <algorithm>
#include <cctype>
#include <sstream>

namespace
{
std::vector<std::string> splitWhitespace(const std::string& text)
{
    std::istringstream stream(text);
    std::vector<std::string> tokens;
    std::string token;
    while (stream >> token)
    {
        tokens.push_back(token);
    }
    return tokens;
}
}

ReceptionParsedEntry ReceptionParser::parseGroups(const std::string& text, char separator)
{
    ReceptionParsedEntry parsed;
    if (text.empty())
        return parsed;

    if (text.find(separator) == std::string::npos)
    {
        parsed.groups = splitWhitespace(text);
    }
    else
    {
        std::string current;
        for (char c : text)
        {
            if (c == separator)
            {
                parsed.groups.push_back(trim(current));
                current.clear();
            }
            else
            {
                current.push_back(c);
            }
        }
        parsed.groups.push_back(trim(current));
    }

    for (std::size_t i = 0; i < parsed.groups.size(); ++i)
    {
        parsed.groups[i] = normalizeGroup(parsed.groups[i]);
        if (parsed.groups[i].size() != 5)
            parsed.invalidIndices.push_back(i);
    }

    return parsed;
}

ReceptionRowEvaluation ReceptionParser::evaluate(const std::vector<std::string>& expectedGroups, const std::string& enteredText, char separator)
{
    ReceptionRowEvaluation evaluation = evaluateGroups(expectedGroups, parseGroups(enteredText, separator).groups);
    evaluation.parsed = parseGroups(enteredText, separator);
    evaluation.incompleteGroupCount = static_cast<int>(evaluation.parsed.invalidIndices.size());
    return evaluation;
}

ReceptionRowEvaluation ReceptionParser::evaluateGroups(const std::vector<std::string>& expectedGroups, const std::vector<std::string>& enteredGroups)
{
    ReceptionRowEvaluation evaluation;
    evaluation.parsed.groups.reserve(enteredGroups.size());

    const std::size_t expectedCount = expectedGroups.size();
    for (std::size_t i = 0; i < enteredGroups.size(); ++i)
    {
        const std::string normalized = normalizeGroup(enteredGroups[i]);
        evaluation.parsed.groups.push_back(normalized);
        if (i < expectedCount && normalized.size() != 5)
        {
            evaluation.parsed.invalidIndices.push_back(i);
            evaluation.incompleteGroupCount += 1;
        }
    }

    const std::size_t maxCount = std::max(expectedCount, enteredGroups.size());
    for (std::size_t i = 0; i < maxCount; ++i)
    {
        const std::string expected = i < expectedCount ? normalizeGroup(expectedGroups[i]) : std::string();
        const std::string actual = i < evaluation.parsed.groups.size() ? evaluation.parsed.groups[i] : std::string();
        const int errors = countGroupErrors(expected, actual);
        evaluation.errorCount += errors;
        if (i < expectedCount && errors == 0 && !expected.empty())
            evaluation.correctGroupCount += 1;
    }

    return evaluation;
}

std::string ReceptionParser::normalizeForCell(const std::string& value)
{
    return normalizeGroup(value);
}

std::string ReceptionParser::trim(const std::string& value)
{
    std::size_t start = 0;
    while (start < value.size() && std::isspace(static_cast<unsigned char>(value[start])))
        ++start;

    std::size_t end = value.size();
    while (end > start && std::isspace(static_cast<unsigned char>(value[end - 1])))
        --end;

    return value.substr(start, end - start);
}

std::string ReceptionParser::normalizeGroup(const std::string& value)
{
    std::string normalized;
    normalized.reserve(value.size());
    for (char c : value)
    {
        if (!std::isspace(static_cast<unsigned char>(c)))
            normalized.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(c))));
    }
    return normalized;
}

int ReceptionParser::countGroupErrors(const std::string& expected, const std::string& actual)
{
    const std::size_t maxLength = std::max(expected.size(), actual.size());
    int errors = 0;
    for (std::size_t i = 0; i < maxLength; ++i)
    {
        const char left = i < expected.size() ? expected[i] : '\0';
        const char right = i < actual.size() ? actual[i] : '\0';
        if (left != right)
            ++errors;
    }
    return errors;
}
