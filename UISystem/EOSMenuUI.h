#if EOSBUILD
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

			UIElementsGroup* lobbySearchField = new UIElementsGroup(ImVec2(0.4f, 0.1f), ImVec2(0.2f, 0.05f), 1.0f, "Lobby Search Field", 0.0f,
				ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar);

		protected:
			std::string inputText;
			int menuOption = eosNone;
			enum menuOptions { eosNone, hostLobby, joinLobby };
		};
	}
}
#endif