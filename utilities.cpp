#include "Utilities.h"
#include <sstream>
#include <algorithm>
#include <utility>
#include <string>
#include <vector>

using namespace std;
std::vector<std::string> stringSplit(const std::string& str, char delim) {
    std::stringstream sstream(str);
    std::string token;
    std::vector<std::string> tokens;
    while(std::getline(sstream, token, delim)) tokens.push_back(token);
    return tokens;
}
string toUpper(const string& str) {
    string ret = str;
    std::transform(ret.begin(), ret.end(), ret.begin(), toupper);
    return ret;
}
string toLower(const string& str) {
    string ret = str;
    std::transform(ret.begin(), ret.end(), ret.begin(), tolower);
    return ret; 
}
bool isUpper(const string& str) {
    return all_of(str.begin(), str.end(), isupper);
}
bool isLower(const string& str) {
    return all_of(str.begin(), str.end(), islower);
}
string reversedString(const string& str) {
    string ret = str;
    reverse(ret.begin(), ret.end());
    return ret;
}
bool randomBool() {
    return rand()%2 == 0;
}
long long randominteger(long long min, long long max) {
    srand(time(NULL));
    return min+(rand())%max+1;
}
