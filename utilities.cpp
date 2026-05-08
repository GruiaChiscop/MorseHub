#include "Utilities.h"
#include <sstream>
#include <algorithm>
#include <utility>
#include <string>
#include <vector>
#include <filesystem>
#include <cstdlib>
#include <random>
#include <cmath>
#ifdef _WIN32
#include <Windows.h>
#endif

using namespace std;
namespace Utils
{
    std::string currentWD;                                    // will store the current directory, usualy the path where MorseHub is installed
    std::string appDataPath;                                  // on windows c:\users\<user>\appdata\roaming, on other systems I'm not decided yet what to choose as a config pat maybe it's $HOME/config.
    std::unique_ptr<MorseGenerator> morseGenerator = nullptr; // a global MorseGenerator for basic things like transmiting 73 at the end of the program. I'm initializing this to nullptr just because I want it to take the the settings from the first setup.
    vector<string> stringSplit(const string &str, char delim)
    {
        stringstream sstream(str);
        string token;
        vector<std::string> tokens;
        while (getline(sstream, token, delim))
            tokens.push_back(token);

        return tokens;
    }
    string toUpper(const string &str)
    {
        string ret = str;
        std::transform(ret.begin(), ret.end(), ret.begin(), [](char c)
                       { return toupper(c); });
        return ret;
    }
    string toLower(const string &str)
    {
        string ret = str;
        std::transform(ret.begin(), ret.end(), ret.begin(), [](char c)
                       { return tolower(c); });
        return ret;
    }
    bool isUpper(const string &str)
    {
        return all_of(str.begin(), str.end(), [](char c)
                      { return isupper(c); });
    }
    bool isLower(const string &str)
    {
        return all_of(str.begin(), str.end(), [](char c)
                      { return islower(c); });
    }
    string reversedString(const string &str)
    {
        string ret = str;
        reverse(ret.begin(), ret.end());
        return ret;
    }
    bool randomBool()
    {
        return rand() % 2 == 0;
    }

    long long randomInteger(long long min, long long max)
    {
        if (max <= min)
            return min;

        static std::mt19937_64 rng{std::random_device{}()};
        std::uniform_int_distribution<long long> distribution(min, max - 1);
        return distribution(rng);
    }

    int speedToDisplayUnits(int cpm, SpeedDisplayMode mode)
    {
        if (mode == SpeedDisplayMode::Wpm)
            return static_cast<int>(std::lround(static_cast<double>(cpm) / 5.0));
        return cpm;
    }

    int displayUnitsToCpm(int value, SpeedDisplayMode mode)
    {
        if (mode == SpeedDisplayMode::Wpm)
            return value * 5;
        return value;
    }

    wxString speedUnitLabel(SpeedDisplayMode mode)
    {
        return mode == SpeedDisplayMode::Wpm ? "WPM" : "CPM";
    }

    wxString formatSpeed(int cpm, SpeedDisplayMode mode, bool includeUnit)
    {
        const int value = speedToDisplayUnits(cpm, mode);
        if (includeUnit)
            return wxString::Format("%d %s", value, speedUnitLabel(mode));
        return wxString::Format("%d", value);
    }

    void say(const string &text, bool wait)
    {
        if (!morseGenerator)
            return;
        if (wait)
            morseGenerator->transmit(text);
        else
            morseGenerator->transmitAsync(text);
    }
    void configureGlobalMorseGenerator(const User& user)
    {
        const auto outputDevice = user.preferredOutputDeviceIndex >= 0
            ? std::optional<AudioOutputDevice>(AudioOutputDeviceService::resolvePlaybackDevice(user.preferredOutputDeviceIndex))
            : std::nullopt;
        morseGenerator = std::make_unique<MorseGenerator>(user.defaultSpeed, user.defaultPitch, user.signalType, outputDevice);
    }
    string getAppData()
    {
#ifdef _WIN32
        char *data = getenv("APPDATA");
        if (data)
            return string(data);
        return ""; // will throw an error instead later
#else
        return ""; // not implemented yet
#endif
    }
    string getWD()
    {
        std::filesystem::path p = std::filesystem::current_path();
        return p.string();
    }
    void cwd(const string &newdir)
    {
        filesystem::path p(newdir);
        filesystem::current_path(p);
    }
}
