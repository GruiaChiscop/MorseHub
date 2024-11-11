/*StringGen.h: The class that is responsible for string generation*/
#ifndef STRINGGEN_H
#define STRINGGEN_H
#include <string>
#include <sstream>
class StringGen
{
    const std::string chars="abcdefghijklmnopqrstuvwxyz1234567890.,/?=+";
     public:
     std::string generate(int num);
     std::string generateonlyLetters(int num);
     std::string generateOnlyDigits(int num);

};
#endif