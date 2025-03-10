#pragma once
#include "../OpenGLRendering/OGLRenderer.h"
#include "../AudioEngine/AudioEngine.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_opengl3.h"
#include "imgui.h"
#include <list>

namespace NCL {
	namespace CSC8508 {

		class UISystem {

			std::list<int> uiList;

		public:
			UISystem(HWND handle);
			~UISystem();

			int GetMenuOption() {
				return menuOption;
			}

			void UpdateFramerate(float delta) {
				dt = delta;
			}

			void StartFrame();
			void EndFrame();
			void DisplayWindow(int window);
			void HideWindow(int window);
			void DrawWindows();

			enum uiElements { framerate, mainMenu, audioSliders };

		protected:

			void DrawDemo();
			void DisplayFramerate();
			void AudioSliders();
			void MainMenu();

			HWND uiWindow;
			bool showDemo = true;

			float masterVolume = 100;
			float musicVolume = 100;
			float sfxVolume = 100;
			float voiceVolume = 100;

			float dt = 0;

			int menuOption = 0;
			enum menuOptions { none, startOffline, startServer, StartClient };

			AudioEngine* audioEngine;
		};
	}
}

