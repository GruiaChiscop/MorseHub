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
MorseGenerator::MorseGenerator(int speed, float frequency, SType type) : m_speed{speed}, m_frequency{frequency}, m_signalType{type}, gen{frequency, 44100, type}
{
    contextConfig = ma_context_config_init();
    if (ma_context_init(NULL, 0, &contextConfig, &context) != MA_SUCCESS)
    {
        throw std::runtime_error("Could not create the context");
        return;
    }
    deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format = ma_format_f32;
    deviceConfig.playback.channels = 1;
    deviceConfig.sampleRate = gen.sampleRate;
    deviceConfig.dataCallback = data_callback;
    deviceConfig.pUserData = this;
    if (ma_device_init(&context, &deviceConfig, &device) != MA_SUCCESS)
    {
        throw std::runtime_error("Failed to initialize the device");
        return;
    }
}

MorseGenerator::~MorseGenerator()
{
    ma_context_uninit(&context);
    ma_device_uninit(&device);
}

std::string MorseGenerator::getOutput(const std::string &text, char separator)
{
    std::ostringstream result;
    ;
    for (char c : text)
    {
        if (characters.count(c))
            result << characters.at(c) << ' ';
        else if (c == ' ')
            result << separator << ' ';
    }
    return result.str();
}

void MorseGenerator::outputToFile(const std::string &file, const std::string &text, char separator)
{
    std::ofstream fout(file);
    fout << getOutput(text, separator);
    fout.close();
}

void MorseGenerator::addCharacter(char c)
{
    if (characters.count(c))
    {
        for (char ch : characters.at(c))
        {
            if (ch == '-')
                gen.add(dashLength(), outputBuffer);
            else if (ch == '.')
                gen.add(unitLength(), outputBuffer);
            gen.addSilence(unitLength(), outputBuffer);
        }
    }
    gen.addSilence(spaceBetweenCharacters(), outputBuffer);
}

void MorseGenerator::addWord(const std::string &word)
{
    for (char c : word)
    {
        addCharacter(c);
    }
    gen.addSilence(spaceBetweenWords(), outputBuffer);
}

void MorseGenerator::transmit(const std::string &text)
{
    std::vector<std::string> words = Utils::stringSplit(text, ' ');
    for (const std::string &word : words)
        addWord(word);

    gen.addSilence(spaceBetweenWords(), outputBuffer);
    pos = 0;
    if (ma_device_start(&device) != MA_SUCCESS)
    {
        throw std::runtime_error("Could not start the device");
        ma_context_uninit(&context);
        ma_device_uninit(&device);
        return;
    }
    std::this_thread::sleep_for(std::chrono::seconds(outputBuffer.size() / deviceConfig.sampleRate));
    outputBuffer.clear();
}

void MorseGenerator::transmitAsync(const std::string &text)
{
    std::thread t([this, text]()
                  { transmit(text); });
    t.detach();
}
std::vector<std::string> MorseGenerator::getOutputDevices() const
{
    std::vector<std::string> devices;
    ma_device_info *playbackDevices;
    ma_uint32 playbackCount;
    if (ma_context_get_devices(const_cast<ma_context *>(&context), &playbackDevices, &playbackCount, NULL, NULL) == MA_SUCCESS)
    {
        for (ma_uint32 i = 0; i < playbackCount; i++)
            devices.emplace_back(playbackDevices[i].name);
    }
    else
    {
        throw std::runtime_error("Could not enumerate devices");
    }
    return devices;
}
void MorseGenerator::setOutputDevice(int deviceIndex)
{
    ma_device_info *pDevices;
    ma_uint32 playbackCount;
    if (ma_context_get_devices(const_cast<ma_context *>(&context), &pDevices, &playbackCount, NULL, NULL) != MA_SUCCESS)
        throw std::runtime_error("Failed to get playback devices");
    if (deviceIndex < 0 || deviceIndex > playbackCount)
        throw std::runtime_error("Index out of bounds");
    ma_device_state deviceState = ma_device_get_state(&device);
    if (deviceState != ma_device_state_uninitialized)
        throw std::runtime_error("Device must be uninitialized");
    deviceConfig.playback.pDeviceID = &pDevices[deviceIndex].id;
}

void MorseGenerator::toFile(const std::string &file, const std::string &text)
{
    ma_encoder encoder;
    ma_encoder_config encoderConfig = ma_encoder_config_init(ma_encoding_format_wav, deviceConfig.playback.format, 2, deviceConfig.sampleRate);
    if (ma_encoder_init_file(file.c_str(), &encoderConfig, &encoder) != MA_SUCCESS)
        throw std::runtime_error("Failed to initialize wave encoder");

    std::vector<std::string> words = Utils::stringSplit(text, ' ');
    for (const std::string &word : words)
        addWord(word);

    gen.addSilence(spaceBetweenWords(), outputBuffer);
    if (ma_encoder_write_pcm_frames(&encoder, outputBuffer.data(), outputBuffer.size(), NULL) != MA_SUCCESS)
    {
        ma_encoder_uninit(&encoder);
        throw std::runtime_error("Could not write PCM frames to the file");
    }
    ma_encoder_uninit(&encoder);
}