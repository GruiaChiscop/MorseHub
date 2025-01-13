/*Speech.h: Speech system header*/

#pragma once
#include <string>
#include <vector>
#ifdef _WIN32
#include "blastspeak.h"
#endif

class TTSVoice {
#ifdef _WIN32
blastspeak* inst;
#endif
int rate, pitch, volume, sampleRate;

public:
TTSVoice();
TTSVoice(const std::string& voiceName);
bool speak(const std::string& text, bool interrupt=true);
bool speakWait(const std::string& text, bool interrupt=true);
std::string toMemory(const std::string& text);
int getRate() const { return rate; }
void setRate(int rate) { this->rate=rate; }
int getVolume() const { return volume; }
void setVolume(int volume) { this->volume=volume; }
int getPitch() const { return pitch; }
void setPitch(int pitch) { this->pitch = pitch; }
int getSampleRate() const { return sampleRate; }
void setSampleRate(int sampleRate) { this->sampleRate=sampleRate; }
};