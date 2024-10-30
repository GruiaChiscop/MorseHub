/*Signal generator class
*This class is used to generate audio signals, which will be used later to play morse code
*This class can generate four types of signals and these are:
*Sinewave, square, triangle and Sawtooth
*/
#ifndef SIGNGEN_H
#define SIGNGEN_H

#include <cmath>
#include <vector>
#include <functional>
class Generator {
const float a = 0.5;
const double pi = 3.14159265358979323846;
std::vector<float> buffer;
public:
float frequency;
int sampleRate;
Generator(float freq= 440.0f, int samplerate = 44100);
~Generator();
void add(float duration, std::function<double(int sampleIndex)> waveformFormula, std::vector<float>& buffer);
void addSquareWave(float duration, std::vector<float>& buffer);
void addSineWave(float duration, std::vector<float>& buffer);
void addTriangleWave(float duration, std::vector<float>& buffer);
void addSawtoothWave(float duration, std::vector<float>& buffer);
void addSilence(float duration, std::vector<float>& buffer);
std::vector<float>& getBuffer() { return buffer; }
void reset();

double sineWave(int sampleIndex) {
    double t = static_cast<double>(sampleIndex)/sampleRate;
    return a*std::sin(2.0*pi*frequency*t);
}
double squareWave(int sampleIndex) {
    double t = static_cast<double>(sampleIndex)/sampleRate;
    return a*(std::sin(2.0*pi*frequency*t)>=0.0?1.0:-0.0);
}
double triangleWave(int sampleIndex) {
    double t = static_cast<double>(sampleIndex)/sampleRate;
    return a*(2.0*std::abs(2.0*(frequency*t-std::floor(0.+frequency*t)))-1.0);
}
double sawtoothWave(int sampleIndex) {
    double t = static_cast<double>(sampleIndex)/sampleRate;
    return a*(2.0*(frequency*t-std::floor(0.5+frequency*t)));
}
};
#endif