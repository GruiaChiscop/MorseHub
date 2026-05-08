#include "SessionHistory.h"

#include <fstream>
#include <stdexcept>

#include "nlohmann/json.hpp"

using nlohmann::json;

namespace
{
int runnerResultKindToInt(RunnerResultKind kind)
{
    return static_cast<int>(kind);
}

RunnerResultKind runnerResultKindFromInt(int value)
{
    return static_cast<RunnerResultKind>(value);
}
}

void to_json(json& j, const Result& result)
{
    j = json{
        {"id", result.id},
        {"text", result.text},
        {"typedText", result.typedText},
        {"runnerEnteredCall", result.runnerEnteredCall},
        {"runnerEnteredRst", result.runnerEnteredRst},
        {"runnerEnteredNr", result.runnerEnteredNr},
        {"runnerSentText", result.runnerSentText},
        {"possiblePoints", result.possiblePoints},
        {"points", result.points},
        {"elapsedMilliseconds", result.elapsedMilliseconds},
        {"speed", result.speed},
        {"frequency", result.frequency},
        {"signalType", static_cast<int>(result.signalType)},
        {"usedRepeat", result.usedRepeat},
        {"errorCount", result.errorCount},
        {"runnerResultKind", runnerResultKindToInt(result.runnerResultKind)},
        {"runnerResponseText", result.runnerResponseText},
        {"runnerRetransmittedText", result.runnerRetransmittedText},
        {"runnerStateText", result.runnerStateText}
    };
}

void from_json(const json& j, Result& result)
{
    if (j.contains("id"))
        j.at("id").get_to(result.id);
    if (j.contains("text"))
        j.at("text").get_to(result.text);
    if (j.contains("typedText"))
        j.at("typedText").get_to(result.typedText);
    if (j.contains("runnerEnteredCall"))
        j.at("runnerEnteredCall").get_to(result.runnerEnteredCall);
    if (j.contains("runnerEnteredRst"))
        j.at("runnerEnteredRst").get_to(result.runnerEnteredRst);
    if (j.contains("runnerEnteredNr"))
        j.at("runnerEnteredNr").get_to(result.runnerEnteredNr);
    if (j.contains("runnerSentText"))
        j.at("runnerSentText").get_to(result.runnerSentText);
    if (j.contains("possiblePoints"))
        j.at("possiblePoints").get_to(result.possiblePoints);
    if (j.contains("points"))
        j.at("points").get_to(result.points);
    if (j.contains("elapsedMilliseconds"))
        j.at("elapsedMilliseconds").get_to(result.elapsedMilliseconds);
    if (j.contains("speed"))
        j.at("speed").get_to(result.speed);
    if (j.contains("frequency"))
        j.at("frequency").get_to(result.frequency);
    if (j.contains("signalType"))
        result.signalType = static_cast<SType>(j.at("signalType").get<int>());
    if (j.contains("usedRepeat"))
        j.at("usedRepeat").get_to(result.usedRepeat);
    if (j.contains("errorCount"))
        j.at("errorCount").get_to(result.errorCount);
    if (j.contains("runnerResultKind"))
        result.runnerResultKind = runnerResultKindFromInt(j.at("runnerResultKind").get<int>());
    if (j.contains("runnerResponseText"))
        j.at("runnerResponseText").get_to(result.runnerResponseText);
    if (j.contains("runnerRetransmittedText"))
        j.at("runnerRetransmittedText").get_to(result.runnerRetransmittedText);
    if (j.contains("runnerStateText"))
        j.at("runnerStateText").get_to(result.runnerStateText);
}

void to_json(json& j, const SessionRecord& session)
{
    j = json{
        {"mode", session.mode},
        {"startedAt", session.startedAt},
        {"rounds", session.rounds},
        {"totalPoints", session.totalPoints},
        {"totalErrors", session.totalErrors},
        {"maxSpeed", session.maxSpeed},
        {"repeatCount", session.repeatCount},
        {"results", session.results}
    };
}

void from_json(const json& j, SessionRecord& session)
{
    if (j.contains("mode"))
        j.at("mode").get_to(session.mode);
    if (j.contains("startedAt"))
        j.at("startedAt").get_to(session.startedAt);
    if (j.contains("rounds"))
        j.at("rounds").get_to(session.rounds);
    if (j.contains("totalPoints"))
        j.at("totalPoints").get_to(session.totalPoints);
    if (j.contains("totalErrors"))
        j.at("totalErrors").get_to(session.totalErrors);
    if (j.contains("maxSpeed"))
        j.at("maxSpeed").get_to(session.maxSpeed);
    if (j.contains("repeatCount"))
        j.at("repeatCount").get_to(session.repeatCount);
    if (j.contains("results"))
        j.at("results").get_to(session.results);
}

std::string SessionHistoryStore::defaultPath()
{
    return "SessionHistory.json";
}

std::vector<SessionRecord> SessionHistoryStore::load(const std::string& path)
{
    std::ifstream file(path);
    if (!file.is_open())
        return {};

    json document;
    file >> document;
    if (!document.is_array())
        throw std::runtime_error("Session history file is invalid");

    std::vector<SessionRecord> sessions;
    sessions.reserve(document.size());
    for (const json& entry : document)
    {
        sessions.push_back(entry.get<SessionRecord>());
    }

    return sessions;
}

void SessionHistoryStore::save(const std::vector<SessionRecord>& sessions, const std::string& path)
{
    std::ofstream file(path);
    if (!file.is_open())
        throw std::runtime_error("Could not open session history file for writing");

    json document = json::array();
    for (const SessionRecord& session : sessions)
    {
        document.push_back(session);
    }
    file << document.dump(4);
}

void SessionHistoryStore::append(const SessionRecord& session, const std::string& path)
{
    std::vector<SessionRecord> sessions = load(path);
    sessions.push_back(session);
    save(sessions, path);
}

void SessionHistoryStore::exportTo(const std::string& path)
{
    save(load(defaultPath()), path);
}

void SessionHistoryStore::importFrom(const std::string& path)
{
    std::vector<SessionRecord> existing = load(defaultPath());
    std::vector<SessionRecord> imported = load(path);
    existing.insert(existing.end(), imported.begin(), imported.end());
    save(existing, defaultPath());
}
