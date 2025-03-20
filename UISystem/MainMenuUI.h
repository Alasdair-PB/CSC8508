#pragma once
#include "imgui.h"
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

			UIElementsGroup* menuUI = new UIElementsGroup(ImVec2(100, 250), ImVec2(500, 500), 1.0f, "MainMenu", 50.0f,
				ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize);

		protected:
			int menuOption = none;
			enum menuOptions { none, startOffline, startServer, startClient };
		};
	}
}