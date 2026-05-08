#include "User.h"
#include "nlohmann/json.hpp"
#include <fstream>
#include <stdexcept>
using nlohmann::json;
//implement the serialization logic for each struct
//wxDateTime:
void to_json(json& j, const wxDateTime& dt)
{
    j = dt.FormatISODate();
}
void from_json(const json& j, wxDateTime& dt)
{
    wxString dateString = j.get<std::string>();
    if(!dt.ParseISODate(dateString))
    {
        //this will throw an error later
        return;
    }

}
//User struct:

void to_json(json& j, const RufzTrainerSettings& settings)
{
    j = json{
        {"fixedSpeed", settings.fixedSpeed},
        {"speed", settings.speed},
        {"adaptiveStep", settings.adaptiveStep},
        {"minSpeed", settings.minSpeed},
        {"groupLength", settings.groupLength},
        {"allowedSymbols", settings.allowedSymbols}
    };
}

void from_json(const json& j, RufzTrainerSettings& settings)
{
    if (j.contains("fixedSpeed"))
        j.at("fixedSpeed").get_to(settings.fixedSpeed);
    if (j.contains("speed"))
        j.at("speed").get_to(settings.speed);
    if (j.contains("adaptiveStep"))
        j.at("adaptiveStep").get_to(settings.adaptiveStep);
    if (j.contains("minSpeed"))
        j.at("minSpeed").get_to(settings.minSpeed);
    if (j.contains("groupLength"))
        j.at("groupLength").get_to(settings.groupLength);
    if (j.contains("allowedSymbols"))
        j.at("allowedSymbols").get_to(settings.allowedSymbols);
}

void to_json(json& j, const RufzSettings& settings)
{
    j = json{
        {"maxRounds", settings.maxRounds},
        {"allowRepeat", settings.allowRepeat},
        {"speedStepCpm", settings.speedStepCpm},
        {"minSpeedCpm", settings.minSpeedCpm},
        {"trainer", settings.trainer}
    };
}

void from_json(const json& j, RufzSettings& settings)
{
    if (j.contains("maxRounds"))
        j.at("maxRounds").get_to(settings.maxRounds);
    if (j.contains("allowRepeat"))
        j.at("allowRepeat").get_to(settings.allowRepeat);
    if (j.contains("speedStepCpm"))
        j.at("speedStepCpm").get_to(settings.speedStepCpm);
    if (j.contains("minSpeedCpm"))
        j.at("minSpeedCpm").get_to(settings.minSpeedCpm);
    if (j.contains("trainer"))
        j.at("trainer").get_to(settings.trainer);
}

void to_json(json& j, const ReceptionSettings& settings)
{
    j = json{
        {"fixedSpeed", settings.fixedSpeed},
        {"speed", settings.speed},
        {"groupLength", settings.groupLength},
        {"allowedSymbols", settings.allowedSymbols},
        {"speeds", settings.speeds},
        {"useInternalInput", settings.useInternalInput},
        {"resultEntryMinutes", settings.resultEntryMinutes},
        {"transmissionSeconds", settings.transmissionSeconds},
        {"pauseSeconds", settings.pauseSeconds},
        {"groupSeparator", settings.groupSeparator},
        {"pitch", settings.pitch},
        {"signalType", static_cast<int>(settings.signalType)}
    };
}

void from_json(const json& j, ReceptionSettings& settings)
{
    if (j.contains("fixedSpeed"))
        j.at("fixedSpeed").get_to(settings.fixedSpeed);
    if (j.contains("speed"))
        j.at("speed").get_to(settings.speed);
    if (j.contains("groupLength"))
        j.at("groupLength").get_to(settings.groupLength);
    if (j.contains("allowedSymbols"))
        j.at("allowedSymbols").get_to(settings.allowedSymbols);
    if (j.contains("speeds"))
        j.at("speeds").get_to(settings.speeds);
    if (j.contains("useInternalInput"))
        j.at("useInternalInput").get_to(settings.useInternalInput);
    if (j.contains("resultEntryMinutes"))
        j.at("resultEntryMinutes").get_to(settings.resultEntryMinutes);
    if (j.contains("transmissionSeconds"))
        j.at("transmissionSeconds").get_to(settings.transmissionSeconds);
    if (j.contains("pauseSeconds"))
        j.at("pauseSeconds").get_to(settings.pauseSeconds);
    if (j.contains("groupSeparator"))
        j.at("groupSeparator").get_to(settings.groupSeparator);
    if (j.contains("pitch"))
        j.at("pitch").get_to(settings.pitch);
    if (j.contains("signalType"))
        settings.signalType = static_cast<SType>(j.at("signalType").get<int>());
}

void to_json(json& j, const RunnerSettings& settings)
{
    j = json{
        {"initialSpeed", settings.initialSpeed},
        {"mode", static_cast<int>(settings.mode)},
        {"competitionMinutes", settings.competitionMinutes},
        {"enableQrm", settings.enableQrm},
        {"enableQrn", settings.enableQrn},
        {"enableQsb", settings.enableQsb},
        {"enableFlutter", settings.enableFlutter},
        {"enableLids", settings.enableLids}
    };
}

void from_json(const json& j, RunnerSettings& settings)
{
    if (j.contains("initialSpeed"))
        j.at("initialSpeed").get_to(settings.initialSpeed);
    if (j.contains("mode"))
        settings.mode = static_cast<RunnerMode>(j.at("mode").get<int>());
    if (j.contains("competitionMinutes"))
        j.at("competitionMinutes").get_to(settings.competitionMinutes);
    if (j.contains("enableQrm"))
        j.at("enableQrm").get_to(settings.enableQrm);
    if (j.contains("enableQrn"))
        j.at("enableQrn").get_to(settings.enableQrn);
    if (j.contains("enableQsb"))
        j.at("enableQsb").get_to(settings.enableQsb);
    if (j.contains("enableFlutter"))
        j.at("enableFlutter").get_to(settings.enableFlutter);
    if (j.contains("enableLids"))
        j.at("enableLids").get_to(settings.enableLids);
}

void to_json(json& j, const ModuleSettings& settings)
{
    j = json{
        {"rufz", settings.rufz},
        {"reception", settings.reception},
        {"runner", settings.runner}
    };
}

void from_json(const json& j, ModuleSettings& settings)
{
    if (j.contains("rufz"))
        j.at("rufz").get_to(settings.rufz);
    if (j.contains("reception"))
        j.at("reception").get_to(settings.reception);
    if (j.contains("runner"))
        j.at("runner").get_to(settings.runner);
}

void to_json(json& j, const User& user)
{
    j = json{{"name", user.name}, {"callsign", user.callsign}, {"dateOfBirth", user.dateOfBirth}, {"defaultSpeed", user.defaultSpeed}, {"defaultPitch", user.defaultPitch}, {"signalType", static_cast<int>(user.signalType)}, {"speedDisplayMode", static_cast<int>(user.speedDisplayMode)}, {"lastSelectedMode", static_cast<int>(user.lastSelectedMode)}, {"preferredOutputDeviceIndex", user.preferredOutputDeviceIndex}, {"outputVolumePercent", user.outputVolumePercent}, {"moduleSettings", user.moduleSettings}};
}
void from_json(const json& j, User& user)
{
    j.at("name").get_to(user.name);
    j.at("callsign").get_to(user.callsign);
    j.at("dateOfBirth").get_to(user.dateOfBirth);
    j.at("defaultSpeed").get_to(user.defaultSpeed);
    j.at("defaultPitch").get_to(user.defaultPitch);
    user.signalType = static_cast<SType>(j.at("signalType").get<int>());
    if (j.contains("speedDisplayMode"))
        user.speedDisplayMode = static_cast<SpeedDisplayMode>(j.at("speedDisplayMode").get<int>());
    else
        user.speedDisplayMode = SpeedDisplayMode::Cpm;
    if (j.contains("lastSelectedMode"))
        user.lastSelectedMode = static_cast<MainMode>(j.at("lastSelectedMode").get<int>());
    else
        user.lastSelectedMode = MainMode::None;
    if (j.contains("preferredOutputDeviceIndex"))
        j.at("preferredOutputDeviceIndex").get_to(user.preferredOutputDeviceIndex);
    else
        user.preferredOutputDeviceIndex = -1;
    if (j.contains("outputVolumePercent"))
        j.at("outputVolumePercent").get_to(user.outputVolumePercent);
    else
        user.outputVolumePercent = 50;
    if (j.contains("moduleSettings"))
    {
        j.at("moduleSettings").get_to(user.moduleSettings);
    }
    else
    {
        if (j.contains("maxRounds"))
            j.at("maxRounds").get_to(user.moduleSettings.rufz.maxRounds);
        if (j.contains("trainerSettings"))
            j.at("trainerSettings").get_to(user.moduleSettings.rufz.trainer);
    }
}

void serialize(const User& u)
{
    json data = u;
    //here we can simply use a ofstream to save the file
std::ofstream f("User.json");
f<<data.dump(4);
f.close();
}
//this function returns nullptr if the file is invalid or it's missing
void deserialize(User& user)
{
    std::ifstream f("User.json");
    if(!f.is_open()) throw std::runtime_error("Error in opening the file");
    json j;
    f>>j;
    f.close();
    user = j.get<User>();
}
