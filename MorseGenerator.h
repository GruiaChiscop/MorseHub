/* *The morse generator class*
* Currently, I will make the code as minimal as possible, later I will add new functionalities
*/
#ifndef MORSEGENERATOR_H
#define MORSEGENERATOR_H
#include "miniaudio.h" //for audio output
#include "SignalGenerator.h"
#include <string>
#include <fstream>
#include <vector>
#include <unordered_map>
class MorseGenerator
{
    public:
    inline static const std::unordered_map<char, std::string> characters { {'a', ".-"}, {'b', "-..."}, {'c', "-.-."}, {'d', "-.."}, {'e', "."}, {'f', "..-."}, {'g', "--."}, {'h', "...."}, {'i', ".."}, {'j', ".---"}, {'k', "-.-"}, {'l', ".-.."}, {'m', "--"}, {'n', "-."}, {'o', "---"}, {'p', ".--."}, {'q', "--.-"}, {'r', "..."}, {'t', "-"}, {'u', "..-"}, {'v', "...-"}, {'w', ".--"}, {'x', "-..-"}, {'y', "-.--"}, {'z', "--.."}, {'1', ".----"}, {'2', "..---"}, {'3', "...--"}, {'4', "....-"}, {'5', "....."}, {'6', "-...."}, {'7', "--..."}, {'8', "---.."}, {'9', "----."}, {'0', "-----"}, {'?', "..--.."}, {'.', ".-.-.-"}, {'/', "-..-."}, {',', "--..--"}, {'=', "-...-"}, {'+', ".-.-."}};
    MorseGenerator(int speed = 20, float pitch = 440.0f, SType type = Sine);
    ~MorseGenerator();
    void transmit(const std::string& text);
    void transmitAsync(const std::string& text);
    void toFile(const std::string& file, const std::string& text);
    std::string getOutput(const std::string& text, char separator = ' ');
    void outputToFile(const std::string& file, const std::string& text, char separator = ' ');
    private:
    std::vector<float> outputBuffer;
    SType m_signalType = Sine;
int m_speed;
float m_frequency;
Generator gen;
ma_device device;
ma_device_config deviceConfig;
static size_t pos;
float unitLength() const {
return 60.0f/(50.0f*m_speed);
}
float dashLength() const { return unitLength()*3; }
float spaceBetweenUnits() const { return unitLength(); }
float spaceBetweenCharacters() const { return dashLength(); }
float spaceBetweenWords() const { return 7*unitLength(); }
void addCharacter(char c);
void addWord(const std::string& word);
//callback
static void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
    MorseGenerator* self = static_cast<MorseGenerator*>(pDevice->pUserData);
float* output = (float*)pOutput;
for(ma_uint32 i=0; i<frameCount; ++i) {
    if(pos<self->outputBuffer.size()) output[i] = self->outputBuffer[pos++];
    else output[i] = 0.0f;
}
}
};
#endif