#pragma once

#include "miniaudio.h"

#include <string>
#include <vector>

struct AudioOutputDevice
{
    int index{};
    std::string name;
};

class AudioOutputDeviceService
{
public:
    static std::vector<AudioOutputDevice> listPlaybackDevices();
    static AudioOutputDevice resolvePlaybackDevice(int index);
    static AudioOutputDevice resolvePlaybackDevice(const std::vector<AudioOutputDevice>& devices, int index);
    static void applyPlaybackDevice(ma_context& context, ma_device_config& deviceConfig, const AudioOutputDevice& outputDevice);
};
