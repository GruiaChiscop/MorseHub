/* *The morse generator class*
* Currently, I will make the code as minimal as possible, later I will add new functionalities
*/
#pragma once
#include "User.h"
#include "SineWaveGenerator.h"
#include "miniaudio.h" //for audio output
#include <string>
#include <fstream>
#include <vector>
#include <map>
class MorseGenerator
{
    public:
    static std::map<char, std::string> characters { {'a', ".-"}, {'b', "-..."}, {'c', "-.-."}, {'d', "-.."}, {'e', "."}, {'f', "..-."}, {'g', "--."}, {'h', "...."}, {'i', ".."}, {'j', ".---"}, {'k', "-.-"}, {'l', ".-.."}, {'m', "--"}, {'n', "-."}, {'o', "---"}, {'p', ".--."}, {'q', "--.-"}, {'r', "..."}, {'t', "-"}, {'u', "..-"}, {'v', "...-"}, {'w', ".--"}, {'x', "-..-"}, {'y', "-.--"}, {'z', "--.."}};
};