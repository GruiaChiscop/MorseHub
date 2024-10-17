#include "MorseGenerator.h"
static void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
	ma_waveform* wave = (ma_waveform*)pDevice->pUserData;
	ma_waveform_read_pcm_frames(wave, pOutput, frameCount, nullptr);
	(void)pInput;
}
MorseGenerator::MorseGenerator(int cpm, int wpm, int frequency, SignalTypes type) : m_cpm{ cpm }, m_wpm{ wpm }, m_freq{ frequency }, m_type{ type } {
	deviceConfig = ma_device_config_init(ma_device_type_playback);
	deviceConfig.playback.format = ma_format_f32;
	deviceConfig.sampleRate = m_sampleRate;
	deviceConfig.playback.channels = 2;
	deviceConfig.dataCallback = data_callback;
	deviceConfig.pUserData = &waveForm;
	if (ma_device_init(nullptr, deviceConfig, device) != MA_SUCCESS)
	{
		throw runtime_error("Could not initialize the device!");
	}
	waveformConfig = ma_waveform_config_init(device.playback.format, device.playback.channels, device.sampleRate, m_type, 0.2, m_freq);
}