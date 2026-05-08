#include "MorseGenerator.h"
#include <vector>
#include <thread> //for async play
#include <string>
#include <fstream> //for outputing to file
#include <stdexcept>
#include <chrono>
#include <cctype>
MorseGenerator::MorseGenerator(int speed, float frequency, SType type, std::optional<AudioOutputDevice> outputDevice, int sampleRate)
    : m_speed{speed}, m_frequency{frequency}, m_signalType{type}, gen{frequency, sampleRate, type}, m_outputDevice(std::move(outputDevice))
{
    initializeAudio();
}

void MorseGenerator::initializeAudio()
{
    contextConfig = ma_context_config_init();
    if (ma_context_init(NULL, 0, &contextConfig, &context) != MA_SUCCESS)
    {
        throw std::runtime_error("Could not create the context");
    }

    deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format = ma_format_f32;
    deviceConfig.playback.channels = 1;
    deviceConfig.sampleRate = gen.sampleRate;
    deviceConfig.dataCallback = data_callback;
    deviceConfig.pUserData = this;

    if (m_outputDevice.has_value())
    {
        AudioOutputDeviceService::applyPlaybackDevice(context, deviceConfig, *m_outputDevice);
    }

    if (ma_device_init(&context, &deviceConfig, &device) != MA_SUCCESS)
    {
        ma_context_uninit(&context);
        throw std::runtime_error("Failed to initialize the device");
    }

    if (ma_device_start(&device) != MA_SUCCESS)
    {
        ma_device_uninit(&device);
        ma_context_uninit(&context);
        throw std::runtime_error("Could not start the device");
    }

    m_audioInitialized = true;
    m_deviceStarted = true;
}

void MorseGenerator::uninitializeAudio()
{
    if (!m_audioInitialized)
        return;

    if (m_deviceStarted)
    {
        ma_device_stop(&device);
        m_deviceStarted = false;
    }
    ma_device_uninit(&device);
    ma_context_uninit(&context);
    m_audioInitialized = false;
}

MorseGenerator::~MorseGenerator()
{
    joinWorker();
    uninitializeAudio();
}

std::string MorseGenerator::getOutput(const std::string &text, char separator)
{
    std::ostringstream result;
    ;
    for (char c : text)
    {
        const char normalized = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        if (characters.count(normalized))
            result << characters.at(normalized) << ' ';
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

void MorseGenerator::addCharacter(char c, std::vector<float>& buffer)
{
    const char normalized = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    if (characters.count(normalized))
    {
        for (char ch : characters.at(normalized))
        {
            if (ch == '-')
                gen.add(dashLength(), buffer);
            else if (ch == '.')
                gen.add(unitLength(), buffer);
            gen.addSilence(unitLength(), buffer);
        }
    }
    gen.addSilence(spaceBetweenCharacters(), buffer);
}

void MorseGenerator::addWord(const std::string &word, std::vector<float>& buffer)
{
    for (char c : word)
    {
        addCharacter(c, buffer);
    }
    gen.addSilence(spaceBetweenWords(), buffer);
}

void MorseGenerator::transmit(const std::string &text)
{
    transmitBuffer(buildBufferForText(text));
}

std::vector<float> MorseGenerator::buildBufferForText(const std::string& text)
{
    std::lock_guard<std::mutex> lock(m_transmitMutex);
    gen.reset();
    gen.frequency = m_frequency;
    std::vector<float> buffer;
    std::string currentWord;
    for (char c : text)
    {
        if (c == ' ')
        {
            if (!currentWord.empty())
            {
                addWord(currentWord, buffer);
                currentWord.clear();
            }
            else
            {
                gen.addSilence(spaceBetweenWords(), buffer);
            }
            continue;
        }

        currentWord.push_back(c);
    }
    if (!currentWord.empty())
        addWord(currentWord, buffer);

    gen.addSilence(spaceBetweenWords(), buffer);
    return buffer;
}

std::vector<float> MorseGenerator::renderText(const std::string& text)
{
    return buildBufferForText(text);
}

void MorseGenerator::transmitBuffer(std::vector<float> buffer)
{
    {
        std::lock_guard<std::mutex> lock(m_bufferMutex);
        outputBuffer = std::move(buffer);
        m_playbackPosition.store(0);
    }
    const std::size_t expectedSamples = outputBuffer.size();
    m_stopRequested.store(false);
    const auto slice = std::chrono::milliseconds(2);
    while (!m_stopRequested.load() && m_playbackPosition.load() < expectedSamples)
    {
        std::this_thread::sleep_for(slice);
    }
    stopPlayback();
}

void MorseGenerator::transmitAsync(const std::string &text)
{
    joinWorker();
    m_worker = std::thread([this, text]()
                           { transmit(text); });
}

void MorseGenerator::transmitBufferAsync(std::vector<float> buffer)
{
    stopPlayback();
    joinWorker();
    std::lock_guard<std::mutex> lock(m_bufferMutex);
    outputBuffer = std::move(buffer);
    m_playbackPosition.store(0);
}

void MorseGenerator::stopPlayback()
{
    m_stopRequested.store(true);
    std::lock_guard<std::mutex> lock(m_bufferMutex);
    outputBuffer.clear();
    m_playbackPosition.store(0);
    m_stopRequested.store(false);
}

void MorseGenerator::waitForPlaybackStop()
{
    stopPlayback();
    joinWorker();
}

void MorseGenerator::setOutputDevice(const AudioOutputDevice& outputDevice)
{
    waitForPlaybackStop();
    joinWorker();
    m_outputDevice = outputDevice;
    uninitializeAudio();
    initializeAudio();
}

void MorseGenerator::clearOutputDevice()
{
    waitForPlaybackStop();
    joinWorker();
    m_outputDevice.reset();
    uninitializeAudio();
    initializeAudio();
}

void MorseGenerator::toFile(const std::string &file, const std::string &text)
{
    std::lock_guard<std::mutex> lock(m_transmitMutex);
    outputBuffer.clear();
    gen.reset();
    ma_encoder encoder;
    ma_encoder_config encoderConfig = ma_encoder_config_init(ma_encoding_format_wav, deviceConfig.playback.format, 2, deviceConfig.sampleRate);
    if (ma_encoder_init_file(file.c_str(), &encoderConfig, &encoder) != MA_SUCCESS)
        throw std::runtime_error("Failed to initialize wave encoder");

    std::string currentWord;
    for (char c : text)
    {
        if (c == ' ')
        {
            if (!currentWord.empty())
            {
                addWord(currentWord, outputBuffer);
                currentWord.clear();
            }
            else
            {
                gen.addSilence(spaceBetweenWords(), outputBuffer);
            }
            continue;
        }

        currentWord.push_back(c);
    }
    if (!currentWord.empty())
        addWord(currentWord, outputBuffer);

    gen.addSilence(spaceBetweenWords(), outputBuffer);
    if (ma_encoder_write_pcm_frames(&encoder, outputBuffer.data(), outputBuffer.size(), NULL) != MA_SUCCESS)
    {
        ma_encoder_uninit(&encoder);
        throw std::runtime_error("Could not write PCM frames to the file");
    }
    ma_encoder_uninit(&encoder);
    outputBuffer.clear();
}

double MorseGenerator::estimateDurationSeconds(const std::string& text) const
{
    auto characterDuration = [this](char c) {
        const char normalized = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        const auto it = characters.find(normalized);
        if (it == characters.end())
        {
            return 0.0;
        }

        double duration = 0.0;
        for (char symbol : it->second)
        {
            duration += (symbol == '-') ? dashLength() : unitLength();
            duration += unitLength();
        }
        duration += spaceBetweenCharacters();
        return duration;
    };

    double total = 0.0;
    for (char c : text)
    {
        if (c == ' ')
        {
            total += spaceBetweenWords();
        }
        else
        {
            total += characterDuration(c);
        }
    }

    return total + spaceBetweenWords();
}

void MorseGenerator::joinWorker()
{
    if (m_worker.joinable())
        m_worker.join();
}
