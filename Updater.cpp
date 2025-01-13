#include "Updater.h"
#include <sstream>
#include "Poco/JSON/Parser.h"
#include "Poco/Json/Object.h"
#include <string>
#include <fstream>
MorseHubUpdater::MorseHubUpdater(const std::string& mirror):m_mirror{mirror}
{
    //fetch the newest version during initialization
    //also pull the update info if exists and if possible
    //will not start downloading any file but the json including the version and changelog
}
bool MorseHubUpdater::available()
{
return false;
}