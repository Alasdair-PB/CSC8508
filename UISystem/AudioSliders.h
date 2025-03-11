#pragma once
#include "../AudioEngine/AudioEngine.h"
#include "imgui.h"

namespace NCL {
	namespace UI {
		class AudioSliders {
		public:
			AudioSliders();
			~AudioSliders();

			void Draw();

		protected:
			float masterVolume = 100;
			float musicVolume = 100;
			float sfxVolume = 100;
			float voiceVolume = 100;

			AudioEngine* audioEngine;
		};
	}
}
