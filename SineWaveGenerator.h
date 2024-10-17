#pragma once
#include <Tonic.h>
using namespace Tonic;
class SineWaveGenerator
{
public:
	SineWaveGenerator(float seconds);

private:
	Generator gen;
	float duration;
};

