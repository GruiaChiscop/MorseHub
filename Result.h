#pragma once
#include <string>
#include <memory>
#include "MorseGenerator.h"

struct Result
{
std::string text;
std::string typedText;
int points;
int miliseconds;
std::unique_ptr<MorseGenerator> morseGenerator;
Result(const std::string& text="", const std::string& typed="", int points = 0, int miliseconds = 0, std::unique_ptr<MorseGenerator> generator = nullptr);
};