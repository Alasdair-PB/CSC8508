#pragma once
#include "../OpenGLRendering/OGLRenderer.h"
#include "../AudioEngine/AudioEngine.h"
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
			static UISystem* GetInstance() {return instance;}
			static void Shutdown() { if (instance) { delete instance; } }

			int GetMenuOption() {return menuOption;}

			void UpdateFramerate(float delta) {dt = delta;}

			virtual void StartFrame() = 0;
			virtual void EndFrame() = 0;

			void UpdateHealth(int healthVal) {
				health = healthVal;
			}

			void DisplayWindow(int window);
			void HideWindow(int window);
			void DrawWindows();

			enum uiElements { framerate, mainMenu, audioSliders, healthbar };

		protected:
			UISystem();
			virtual ~UISystem();

			static UISystem* instance;

			float dt = 0;
			int health = 50;

			int menuOption = 0;
			enum menuOptions { none, startOffline, startServer, StartClient };
		};
	}
}

