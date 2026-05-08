#pragma once

#include <string>
#include <vector>

struct ReceptionSessionRow
{
    int speedCpm{};
    std::vector<std::string> sentGroups;
    std::string sentTransmission;
    std::string enteredGroups;
};
