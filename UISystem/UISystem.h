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

			int masterVolume = 100;
			int musicVolume = 100;
			int sfxVolume = 100;
			int voiceVolume = 100;
		};
	}
}

