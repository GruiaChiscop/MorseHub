#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include "MorseGenerator.h"
#include "Utilities.h"
#include <vector>
#include <thread> //for async play
#include <string>
#include <fstream> //for outputing to file
#include <stdexcept>
#include <chrono>
size_t MorseGenerator::pos = 0;
MorseGenerator::MorseGenerator(int speed, float pitch, SignalType type) : m_frequency {pitch}, m_speed{speed}, m_signalType{type}, gen { pitch, 44100, static_cast<SType>(type)} {
deviceConfig = ma_device_config_init(ma_device_type_playback);
deviceConfig.playback.format = ma_format_f32;
deviceConfig.playback.channels = 1;
deviceConfig.sampleRate = gen.sampleRate;
deviceConfig.dataCallback = data_callback;
    deviceConfig.pUserData = this;
    if(ma_device_init(nullptr, &deviceConfig, &device)!=MA_SUCCESS) {
        throw std::runtime_error("Failed to initialize the device");
                return;
    }
}

MorseGenerator::~MorseGenerator() {
    ma_device_uninit(&device);
}

std::string MorseGenerator::getOutput(const std::string& text, char separator) {
    std::ostringstream result;;
    for(char c: text) {
if(characters.count(c)) result<<characters.at(c)<<' ';
else if(c == ' ') result<<separator<<' ';
    }
    return result.str();
}

void MorseGenerator::outputToFile(const std::string& file, const std::string& text, char separator) {
    std::ofstream fout(file);
    fout<<getOutput(text, separator);
    fout.close();
}

void MorseGenerator::addCharacter(char c) {
    if(characters.count(c)) {
        for(char ch: characters.at(c)) {
            if(ch=='-') gen.add(dashLength(), outputBuffer);
            else if(ch=='.') gen.add(unitLength(), outputBuffer);
            gen.addSilence(unitLength(), outputBuffer);
        }
    }
        gen.addSilence(spaceBetweenCharacters(), outputBuffer);
}

void MorseGenerator::addWord(const std::string& word) {
for(char c: word) {
    addCharacter(c);
}
gen.addSilence(spaceBetweenWords(), outputBuffer);
}

void MorseGenerator::transmit(const std::string& text) {
    std::vector<std::string> words = Utils::stringSplit(text, ' ');
    for(const std::string& word: words) addWord(word);
    
    gen.addSilence(spaceBetweenWords(), outputBuffer);
    pos = 0;
    if(ma_device_start(&device)!=MA_SUCCESS) {
        throw std::runtime_error("Could not start the device");
        ma_device_uninit(&device);
        return;
    }
    std::this_thread::sleep_for(std::chrono::seconds(outputBuffer.size()/deviceConfig.sampleRate));
    outputBuffer.clear();
}

void MorseGenerator::transmitAsync(const std::string& text) {
std::thread t([this, text]() { transmit(text); });
t.detach();
}