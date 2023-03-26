#include "AudioDevice.h"


namespace Slick::Audio {
	
	int audioCallback(const void *input, void *output, unsigned long frameCount, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData) {
		AudioData* ad = (AudioData*)userData;

		float* out = (float*)output;

		for (u32 i = 0; i < frameCount; i++) {
			float v = sin(ad->t) * 0.2f;
			*(out++) = v;
			*(out++) = v;
			ad->t += 0.03f;
		}
		return 0;
	}

	AudioDevice::AudioDevice() 
		:
		mOutput(nullptr),
		mData({})
	{
		Pa_Initialize();

		Pa_OpenDefaultStream(
			&mOutput,
			0,
			2,
			paFloat32,
			44100,
			256,
			audioCallback,
			(void*)&mData
		);

		Pa_StartStream(mOutput);
	}

	AudioDevice::~AudioDevice() {
		Pa_Terminate();
	}

}
