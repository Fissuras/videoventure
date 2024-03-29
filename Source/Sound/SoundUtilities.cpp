#include "StdAfx.h"

#include "Sound.h"
#include "SoundUtilities.h"

// output a fixed-value pulse of "ticks" duration
// returns the fractional sample time remaining
float OutputPulse(SoundTemplate &self, int ticks, float samplespertick, float samples, short value)
{
	samples += ticks * samplespertick;
	int count = xs_FloorToInt(samples);
	if (count > 0)
	{
		// append samples
		samples -= count;
		self.Reserve(count);
		self.Append(value, count);
	}
	return samples;
}

// 
bool ApplyConstant(float target[], int width, int count, const float keys[], float aTime, int &aIndex)
{
	memcpy(&target[0], &keys[1], width * sizeof(target[0]));
	return true;
}

