#pragma once
#include "../OpenGLRendering/OGLRenderer.h"
#include "../AudioEngine/AudioEngine.h"

namespace NCL {
	namespace CSC8508 {
		class UISystem {
		public:
			UISystem(HWND handle);
			~UISystem();

			void SetWindow(HWND handle) {
				uiWindow = handle;
			}

			void StartFrame();
			void EndFrame();

			void DrawDemo();
			void DisplayFramerate(float dt);
			void AudioSliders();
			int MainMenu();

		protected:
			HWND uiWindow;
			bool showDemo = true;

			float masterVolume = 100;
			float musicVolume = 100;
			float sfxVolume = 100;
			float voiceVolume = 100;

			AudioEngine* audioEngine;
		};
	}
}

