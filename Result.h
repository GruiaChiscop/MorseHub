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
int speed;
int frequency;
std::unique_ptr<MorseGenerator> morseGenerator;
};