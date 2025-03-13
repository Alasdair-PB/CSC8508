#pragma once
#include "../OpenGLRendering/OGLRenderer.h"
#include "../AudioEngine/AudioEngine.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_opengl3.h"
#include "imgui.h"
#include "AudioSliders.h"
#include "FramerateUI.h"
#include "MainMenuUI.h"
#include "Healthbar.h"
#include <list>

namespace NCL {
	namespace UI {

		class UISystem {

			std::list<int> uiList;

		public:
			UISystem(HWND handle);
			~UISystem();

			int GetMenuOption() {
				return menuOption;
			}

			void SetWinSize(int width, int height) {
				winWidth = width;
				winHeight = height;
			}

			void UpdateFramerate(float delta) {
				dt = delta;
			}

			void StartFrame();
			void EndFrame();
			void DisplayWindow(int window);
			void HideWindow(int window);
			void DrawWindows();

			enum uiElements { framerate, mainMenu, audioSliders, healthbar };

		protected:
			HWND uiWindow;

			float dt = 60;
			int health = 50;

			int winWidth = 0;
			int winHeight = 0;

			int menuOption = 0;
			enum menuOptions { none, startOffline, startServer, StartClient };
		};
	}
}

