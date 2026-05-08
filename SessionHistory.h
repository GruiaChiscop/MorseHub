#pragma once

#include <string>
#include <vector>

#include "Result.h"

struct SessionRecord
{
    std::string mode;
    std::string startedAt;
    int rounds{};
    int totalPoints{};
    int totalErrors{};
    int maxSpeed{};
    int repeatCount{};
    std::vector<Result> results;
};

class SessionHistoryStore
{
public:
    static std::string defaultPath();
    static std::vector<SessionRecord> load(const std::string& path = defaultPath());
    static void save(const std::vector<SessionRecord>& sessions, const std::string& path = defaultPath());
    static void append(const SessionRecord& session, const std::string& path = defaultPath());
    static void exportTo(const std::string& path);
    static void importFrom(const std::string& path);
};
