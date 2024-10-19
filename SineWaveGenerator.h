#pragma once
#include <Tonic.h>
#include <vector>
using namespace std;
using namespace Tonic;
class SineWaveGenerator
{
public:
	SineWaveGenerator(float seconds);
	void addSilence(vector<float>& outBuffer, float duration);
	vector<float> generate();
	int sampleRate() const { return m_sampleRate; }
	void setSampleRate(int s) { m_sampleRate = s; }
int frequency() const { return m_frequency; }
private:
	Generator gen;
	float duration;
	vector<float> buffer;
	int m_sampleRate = 44100;
	int m_frequency = 440;
};