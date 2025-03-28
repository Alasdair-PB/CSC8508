#pragma once
#include "UIElementsGroup.h"

namespace NCL {
	namespace UI {

		class MainMenuUI {
		public:
			MainMenuUI();
			~MainMenuUI();

			int GetMenuOption() {
				return menuOption;
			}

			UIElementsGroup* menuUI = new UIElementsGroup(ImVec2(0.1f, 0.5f), ImVec2(1, 1), 2.0f, "MainMenu", 0.05f,
				ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize);

		protected:
			int menuOption = none;
			enum menuOptions { none, startOffline, startServer, startClient, startEOS };
		};
	}
}