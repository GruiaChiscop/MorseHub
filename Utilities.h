#ifndef UTILITIES_H
#define UTILITIES_H
#include <vector>
#include <string>
#include <sstream>
#include <memory>
#include <wx/string.h>
#include "MorseGenerator.h"
#include "User.h"
#include <random>

using std::string;
namespace Utils
{
std::vector<std::string> stringSplit(const std::string& str, char delim);
bool isUpper(const std::string& str);
bool isLower(const std::string& str);
std::string toUpper(const std::string& str);
std::string toLower(const std::string& str);
std::string reversedString(const std::string& str);
bool randomBool();
// Returns a uniformly distributed integer in the half-open interval [min, max).
long long randomInteger(long long min, long long max);
int speedToDisplayUnits(int cpm, SpeedDisplayMode mode);
int displayUnitsToCpm(int value, SpeedDisplayMode mode);
wxString speedUnitLabel(SpeedDisplayMode mode);
wxString formatSpeed(int cpm, SpeedDisplayMode mode, bool includeUnit = true);
extern std::string currentWD; //will store the current directory, usualy the path where MorseHub is installed
extern std::string appDataPath; //on windows c:\users\<user>\appdata\roaming, on other systems I'm not decided yet what to choose as a config pat maybe it's $HOME/config.
extern std::unique_ptr<MorseGenerator> morseGenerator; //a global MorseGenerator for basic things like transmiting 73 at the end of the program. I'm initializing this to nullptr just because I want it to take the the settings from the first setup.
//this function is intended to be used only for general purposes, not in competitions
void say(const string& text, bool wait = false);
void configureGlobalMorseGenerator(const User& user);
string getAppData();
string getWD();
void cwd(const string& newdir);
string getComputerName();
string getComputerUserName();
string getComputerUUID();
int getAllRunningProcesses();
bool lockResources();
bool releaseResources();
bool appIsAlreadyRunning();
bool isAppAdmin(); //for windows only
}
#endif
