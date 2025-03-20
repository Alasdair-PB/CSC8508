#pragma once
#include "imgui.h"

namespace NCL {
	namespace UI {

		class MainMenuUI {
		public:
			MainMenuUI();
			~MainMenuUI();

			int Draw();

			int GetMenuOption() {
				return menuOption;
			}

		protected:
			int menuOption = 0;
			enum menuOptions { none, startOffline, startServer, StartClient, hostRoom, joinRoom };
		};
	}
}