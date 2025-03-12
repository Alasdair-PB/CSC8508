#pragma once
#include "../OpenGLRendering/OGLRenderer.h"
#include "../AudioEngine/AudioEngine.h"

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

			void DisplayWindow(int window);
			void HideWindow(int window);
			void DrawWindows();

			enum uiElements { framerate, mainMenu, audioSliders };

		protected:
			UISystem();
			virtual ~UISystem();

			void DrawDemo();
			void DisplayFramerate();
			void AudioSliders();
			void MainMenu();

			static UISystem* instance;
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

