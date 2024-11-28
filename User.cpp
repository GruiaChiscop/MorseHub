#include "User.h"
#include "nlohmann/json.hpp"
#include <fstream>
#include <stdexcept>
using nlohmann::json;
//implemement the serialization logic for each struct
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

void to_json(json& j, const User& user)
{
    j = json{{"name", user.name}, {"callsign", user.callsign}, {"dateOfBirth", user.dateOfBirth}, {"defaultSpeed", user.defaultSpeed}, {"defaultPitch", user.defaultPitch}, {"signalType", static_cast<int>(user.signalType)}};
}
void from_json(const json& j, User& user)
{
    j.at("name").get_to(user.name);
    j.at("callsign").get_to(user.callsign);
    j.at("dateOfBirth").get_to(user.dateOfBirth);
    j.at("defaultSpeed").get_to(user.defaultSpeed);
    j.at("defaultPitch").get_to(user.defaultPitch);
    user.signalType = static_cast<SType>(j.at("signalType").get<int>());
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
