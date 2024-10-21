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
    enum SignalType
    {
        SignalType_Sine = ma_waveform_type_sine,
        SignalType_Square = ma_waveform_type_square,
        SignalType_Triangle = ma_waveform_type_triangle,
        SignalType_sawtooth = ma_waveform_type_sawtooth
    };
    static std::map<char, std::string> characters { {'a', ".-"}, {'b', "-..."}, {'c', "-.-."}, {'d', "-.."}, {'e', "."}, {'f', "..-."}, {'g', "--."}, {'h', "...."}, {'i', ".."}, {'j', ".---"}, {'k', "-.-"}, {'l', ".-.."}, {'m', "--"}, {'n', "-."}, {'o', "---"}, {'p', ".--."}, {'q', "--.-"}, {'r', "..."}, {'t', "-"}, {'u', "..-"}, {'v', "...-"}, {'w', ".--"}, {'x', "-..-"}, {'y', "-.--"}, {'z', "--.."}, {'1', ".----"}, {'2', "..---"}, {'3', "...--"}, {'4', "....-"}, {'5', "....."}, {'6', "-...."}, {'7', "--..."}, {'8', "---.."}, {'9', "----."}, {'0', "-----"}, {'?', "..--.."}, {'.', ".-.-.-"}, {'/', "-..-."}, {',', "--..--"}, {'=', "-...-"}, {'+', ".-.-."}};
    void transmit(const std::string& text);
    void toFile(const std::string& file, const std::string& text);
    std::string getOutput(const std::string& text, char separator = ' ');
    void outputToFile(const std::string& file, const std::string& outputMorse);
    private:
    std::vector<float> outputBuffer;
    SignalType m_signalType = SignalType_Sine;

};