#include "AudioDevice.h"


namespace Slick::Audio {
	
	int audioCallback(const void *input, void *output, unsigned long frameCount, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData) {
		AudioData* ad = (AudioData*)userData;

		float* out = (float*)output;

		float fac = 0.9f;
		ad->volume = ad->volume * fac + ad->target_volume * (1.f - fac);

		for (u32 i = 0; i < frameCount; i++) {
			float v = sin(ad->t) * ad->volume;
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

		mData = {
			.volume = 0.f,
			.target_volume = .2f,
			.t = 0.f,
		};

		Pa_StartStream(mOutput);
	}

	AudioDevice::~AudioDevice() {
		mute();
		Pa_Sleep(100);
		Pa_Terminate();
	}

	void AudioDevice::mute() {
		if (volume() != 0.f) {
			mMutedVolume = volume();
			volume() = 0.f;
		}
	}

	void AudioDevice::unmute() {
		if(mMutedVolume != 0.f)
			volume() = mMutedVolume;
	}

}
