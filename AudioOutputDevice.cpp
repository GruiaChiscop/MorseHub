#include "AudioOutputDevice.h"

#include <stdexcept>

namespace
{
std::vector<AudioOutputDevice> listPlaybackDevices(ma_context& context)
{
    ma_device_info* playbackDevices = nullptr;
    ma_uint32 playbackCount = 0;
    if (ma_context_get_devices(&context, &playbackDevices, &playbackCount, nullptr, nullptr) != MA_SUCCESS)
        throw std::runtime_error("Could not enumerate playback devices");

    std::vector<AudioOutputDevice> devices;
    devices.reserve(playbackCount);
    for (ma_uint32 i = 0; i < playbackCount; ++i)
    {
        devices.push_back(AudioOutputDevice{static_cast<int>(i), playbackDevices[i].name});
    }

    return devices;
}
}

std::vector<AudioOutputDevice> AudioOutputDeviceService::listPlaybackDevices()
{
    ma_context context;
    ma_context_config contextConfig = ma_context_config_init();
    if (ma_context_init(nullptr, 0, &contextConfig, &context) != MA_SUCCESS)
        throw std::runtime_error("Could not create the audio context");

    try
    {
        std::vector<AudioOutputDevice> devices = ::listPlaybackDevices(context);
        ma_context_uninit(&context);
        return devices;
    }
    catch (...)
    {
        ma_context_uninit(&context);
        throw;
    }
}

AudioOutputDevice AudioOutputDeviceService::resolvePlaybackDevice(int index)
{
    return resolvePlaybackDevice(listPlaybackDevices(), index);
}

AudioOutputDevice AudioOutputDeviceService::resolvePlaybackDevice(const std::vector<AudioOutputDevice>& devices, int index)
{
    for (const AudioOutputDevice& device : devices)
    {
        if (device.index == index)
            return device;
    }

    throw std::out_of_range("Audio output device index out of bounds");
}

void AudioOutputDeviceService::applyPlaybackDevice(ma_context& context, ma_device_config& deviceConfig, const AudioOutputDevice& outputDevice)
{
    ma_device_info* playbackDevices = nullptr;
    ma_uint32 playbackCount = 0;
    if (ma_context_get_devices(&context, &playbackDevices, &playbackCount, nullptr, nullptr) != MA_SUCCESS)
        throw std::runtime_error("Failed to get playback devices");

    const AudioOutputDevice resolved = resolvePlaybackDevice(::listPlaybackDevices(context), outputDevice.index);
    deviceConfig.playback.pDeviceID = &playbackDevices[resolved.index].id;
}
