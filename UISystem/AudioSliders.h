#pragma once
#include "../AudioEngine/AudioEngine.h"
#include "UIElementsGroup.h"

namespace NCL {
	namespace UI {
		class AudioSliders {
		public:
			AudioSliders();
			~AudioSliders();

			UIElementsGroup* audioSlidersUI = new UIElementsGroup(ImVec2(0.6f, 0.3f), ImVec2(0.3f, 0.3f), 1.0f, "Audio Sliders", 0.0f, ImGuiWindowFlags_NoResize);

		protected:
			float masterVolume = 100;
			float musicVolume = 100;
			float sfxVolume = 100;
			float voiceVolume = 100;

			int inputDevice = 0;
			int outputDevice = 0;

			AudioEngine* audioEngine;
		};
	}
}
