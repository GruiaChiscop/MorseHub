#pragma once
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include <vector>
#include <string>
#include <Tonic.h> //for tone generation
#include <fstream>
#include <stdexcept>
using namespace Tonic;
using namespace std;

class MorseGenerator
{
	
public:
	enum SignalTypes {
		SignalType_Sine = ma_waveform_type_sine,
		SignalType_Square=ma_waveform_type_square,
		SignalType_Triangle = ma_waveform_type_triangle,
		SignalType_Sawtoot = ma_waveform_type_sawtooth,
		SignalType_Noise = 4,
		SignalType_Custom = 5 //Tonic supports creating custom tones
	};
	MorseGenerator(int cpm=20, int wpm=20, int frequency=440, SignalTypes type = SignalType_Sine);
	
	void transmit(const string& text);
	void writeToFile(const string& filename, const string& text);
	int cpm() const { return m_cpm; }
	int wpm() const { return m_wpm; }
	int frequency() const { return m_freq; }
	int sampleRate() const { return m_sampleRate; }
	void setCPM(int cpm) { m_cpm = cpm; }
	void setWPM(int wpm)
	{
		if (wpm < m_cpm)
		{
			throw runtime_error("Word speed must be greater than or equal to character speed");
		}
		else
		{
			m_wpm = wpm;
		}
	}
	void setFrequency(int frequency) { m_freq = frequency; }
	void setSampleRate(int sampleRate) { m_sampleRate = sampleRate; }
private:
	ma_device_config deviceConfig;
	ma_waveform_config waveformConfig;
	ma_waveform waveForm;
	ma_device device;
	SignalTypes m_type = SignalType_Sine;
	int m_cpm;
	int m_wpm;
	int m_freq;
	int m_sampleRate = 48000;


};

