#pragma once
#include <string>
#include "Curl/curl.h"
// this class will hold the logic for any update, including application or callsign databases

    class MorseHubUpdater
    {
    
    public:
        MorseHubUpdater(const std::string &mirror);
        bool available();
        void download(const std::string& file);
        std::string getUpdateInfo() const { return m_updateInfo; }
        double size() const { return m_size; }
        const std::string version() const { return m_version; }
    private:
        double m_size;
        std::string m_updateInfo;
        std::string m_mirror;
        const std::string m_version = "1.0.0";
    std::string newVersion;
    };