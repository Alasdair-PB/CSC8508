#pragma once
#include "imgui.h"
#include "UIElementsGroup.h"

namespace NCL {
	namespace UI {

		class EOSMenuUI {
		public:
			EOSMenuUI();
			~EOSMenuUI();

			int GetMenuOption() {
				return menuOption;
			}

			UIElementsGroup* eosMenuUI = new UIElementsGroup(ImVec2(0.1f, 0.5f), ImVec2(1, 1), 2.0f, "EOSMenu", 0.05f,
				ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize);

		protected:
			int menuOption = none;
			enum menuOptions { none, hostLobby, joinLobby };
		};
	}
}