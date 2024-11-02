#ifndef UTILITIES_H
#define UTILITIES_H
#include <vector>
#include <string>
#include <sstream>
std::vector<std::string> stringSplit(const std::string& str, char delim);
bool isUpper(const std::string& str);
bool isLower(const std::string& str);
std::string toUpper(const std::string& str);
std::string toLower(const std::string& str);
std::string reversedString(const std::string& str);
bool randomBool();
long long randomInteger(long long min, long long max);
#endif