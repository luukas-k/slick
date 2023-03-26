#pragma once

#include "Core.h"

#include "portaudio.h"

namespace Slick::Audio {

	struct AudioData {
		float volume, target_volume;

		float t;
	};

	class AudioDevice {
	public:
		AudioDevice();
		~AudioDevice();

		inline float& volume() { return mData.target_volume; }
		void mute();
		void unmute();
	private:
		PaStream* mOutput;
		AudioData mData;
		float mMutedVolume;
	};

}