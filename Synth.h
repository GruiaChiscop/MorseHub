#ifndef SYNTH_H
#define SYNTH_H
#include <vector>
#include <Tonic.h>
class Synth {
private:
    /* data */
    int m_sampleRate = 44100;
    float m_frequency = 440.0f;
Tonic::Synth synth;
    
    std::vector<float> outBuffer;
    Tonic::Generator gen;

public:
enum Type {
        SINE,
        SQUARE,
        TRIANGLE,
        SAWTOOTH
    };
    Synth(Type t = SINE, float frequency = 440.0f, int sampleRate = 44100);
    
void addToBuffer(float seconds);
void addSilence(float seconds);
std::vector<float> buffer() const { return outBuffer; } //required for writting to files and miniaudio playback
void reset();
void setFrequency(int freq) { m_frequency = freq; }
void setSampleRate(int s) {
    m_sampleRate = s;
    Tonic::setSampleRate(s);
    }
int sampleRate() const { return m_sampleRate; }
float frequency() const { return m_frequency; }
};

#endif