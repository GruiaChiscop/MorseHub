/*HTTP.h: HTTP request definition code*/
#pragma once
#include <string>
class HTTPRequest {
    public:
    FILE* stream; //for savind data to file
    std::string response;
HTTPRequest(const std::string& url);
bool perform();
bool perform(const std::string& url);
double m_bytesDownloaded, m_totalSize, m_dlpercentage, m_ulpercentage;
double time;
int code;
std::string m_filePath, url;
bool inProgress, complete, aborted, noCURL;
};