#include "StringGen.h"
#include "Utilities.h"
std::string StringGen::generate(int num)
{
    std::ostringstream ss;
    for(int i=0; i<num; ++i)
    {
        int randIndex = randomInteger(0, chars.length());
        ss<<chars[randIndex];
    }
    return ss.str();
}
std::string StringGen::generateOnlyDigits(int num)
{
    std::ostringstream ss;
    std::string digits;
    for(char c: chars)
    {
        if(isdigit(c)) digits+=c;
    }
    for(int i=0; i<num; ++i)
    {
        int randomIndex = randomInteger(0, digits.length());
        ss<<digits[randomIndex];
    }
    return ss.str();
}

std::string StringGen::generateonlyLetters(int num)
{
    std::ostringstream ss;
    std::string letters = chars.substr(0, 26);
    for(int i=0; i<num; ++i)
    {
        int randomindex = randomInteger(0, letters.size());
        ss<<letters[randomindex];
    }
    return ss.str();
}