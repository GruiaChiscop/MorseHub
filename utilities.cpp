#include "Utilities.h"
#include <sstream>
std::vector<std::string> stringSplit(const std::string& str, char delim) {
    std::stringstream sstream(str);
    std::string token;
    std::vector<std::string> tokens;
    while(std::getline(sstream, token, delim)) tokens.push_back(token);
    return tokens;
}