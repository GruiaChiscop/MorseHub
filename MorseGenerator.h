/* *The morse generator class*
* Currently, I will make the code as minimal as possible, later I will add new functionalities
*/
#ifndef MORSEGENERATOR_H
#define MORSEGENERATOR_H
#include "miniaudio.h" //for audio output
#include "AudioOutputDevice.h"
#include "SignalGenerator.h"
#include <optional>
#include <string>
#include <fstream>
#include <atomic>
#include <mutex>
#include <thread>
#include <vector>
#include <unordered_map>
class MorseGenerator
{
public:
    inline static const std::unordered_map<char, std::string> characters { {'a', ".-"}, {'b', "-..."}, {'c', "-.-."}, {'d', "-.."}, {'e', "."}, {'f', "..-."}, {'g', "--."}, {'h', "...."}, {'i', ".."}, {'j', ".---"}, {'k', "-.-"}, {'l', ".-.."}, {'m', "--"}, {'n', "-."}, {'o', "---"}, {'p', ".--."}, {'q', "--.-"}, {'r', "..."}, {'t', "-"}, {'u', "..-"}, {'v', "...-"}, {'w', ".--"}, {'x', "-..-"}, {'y', "-.--"}, {'z', "--.."}, {'1', ".----"}, {'2', "..---"}, {'3', "...--"}, {'4', "....-"}, {'5', "....."}, {'6', "-...."}, {'7', "--..."}, {'8', "---.."}, {'9', "----."}, {'0', "-----"}, {'?', "..--.."}, {'.', ".-.-.-"}, {'/', "-..-."}, {',', "--..--"}, {'=', "-...-"}, {'+', ".-.-."}};
    MorseGenerator(int speed = 20, float frequency = 440.0f, SType type = Sine, std::optional<AudioOutputDevice> outputDevice = std::nullopt, int sampleRate = 44100);
    ~MorseGenerator();
    void transmit(const std::string& text);
    void transmitAsync(const std::string& text);
    void transmitBuffer(std::vector<float> buffer);
    void transmitBufferAsync(std::vector<float> buffer);
    void stopPlayback();
    void waitForPlaybackStop();
    void toFile(const std::string& file, const std::string& text);
    std::string getOutput(const std::string& text, char separator = ' ');
    void outputToFile(const std::string& file, const std::string& text, char separator = ' ');
    std::vector<float> renderText(const std::string& text);
    void setOutputDevice(const AudioOutputDevice& outputDevice);
    void clearOutputDevice();
    std::optional<AudioOutputDevice> outputDevice() const { return m_outputDevice; }
    double estimateDurationSeconds(const std::string& text) const;
    int getSpeed() const { return m_speed; }
    void setSpeed(int speed) { m_speed = speed; }
    float getFrequency() const { return m_frequency; }
    void setFrequency(float frequency) { m_frequency = frequency; gen.frequency = frequency; }
    SType getSignalType() const { return m_signalType; }
    void setSignalType(SType type) { m_signalType = type; }
    int getSampleRate() const { return gen.sampleRate; }
    private:
    std::vector<float> outputBuffer;
    SType m_signalType = Sine;
    int m_speed;
    float m_frequency;
    Generator gen;
    ma_device device{};
    ma_device_config deviceConfig{};
    ma_context context{}; // used for playback devices
    ma_context_config contextConfig{};
    std::optional<AudioOutputDevice> m_outputDevice;
    bool m_audioInitialized = false;
    bool m_deviceStarted = false;
    std::atomic_size_t m_playbackPosition{0};
    std::atomic_bool m_stopRequested{false};
    std::mutex m_transmitMutex;
    std::mutex m_bufferMutex;
    std::thread m_worker;
    float unitLength() const {
    return 1.2f / m_speed;
    }
    float dashLength() const { return 3.6f / m_speed; }
    float spaceBetweenUnits() const { return unitLength(); }
    float spaceBetweenCharacters() const { return dashLength(); }
    float spaceBetweenWords() const { return 7 * unitLength(); }
    void addCharacter(char c, std::vector<float>& buffer);
    void addWord(const std::string& word, std::vector<float>& buffer);
    std::vector<float> buildBufferForText(const std::string& text);
    void initializeAudio();
    void uninitializeAudio();
    void joinWorker();
//callback
static void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
    MorseGenerator* self = static_cast<MorseGenerator*>(pDevice->pUserData);
float* output = (float*)pOutput;
std::lock_guard<std::mutex> lock(self->m_bufferMutex);
size_t pos = self->m_playbackPosition.load();
for(ma_uint32 i=0; i<frameCount; ++i) {
    if(pos<self->outputBuffer.size()) output[i] = self->outputBuffer[pos++];
    else output[i] = 0.0f;
}
self->m_playbackPosition.store(pos);
}
};
#endif
