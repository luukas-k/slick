#pragma once

#include "Core.h"

#include "portaudio.h"

namespace Slick::Audio {

	struct AudioData {
		float t;
	};

	class AudioDevice {
	public:
		AudioDevice();
		~AudioDevice();

	private:
		PaStream* mOutput;
		AudioData mData;
	};

}