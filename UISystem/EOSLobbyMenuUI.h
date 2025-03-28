#if EOSBUILD

#pragma once
#include "imgui.h"
#include "UIElementsGroup.h"
#include "../EOSFramework/EOSLobbyManager.h"

namespace NCL {
	namespace UI {

		class EOSLobbyMenuUI {
		public:
			EOSLobbyMenuUI(bool lobbyOwner, const std::string& ip, const std::string& lobbyID, int playerCount);
			~EOSLobbyMenuUI();

			int GetMenuOption() {
				return menuOption;
			}

			UIElementsGroup* eosLobbyMenuUI = new UIElementsGroup(ImVec2(0.1f, 0.5f), ImVec2(1, 1), 2.0f, "EOSLobbyMenu", 0.05f,
				ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize);

		protected:

			int menuOption = eosLobbyNone;

			enum menuOptions { eosLobbyNone, startGameAsHost, startGameAsJoin };
		};
	}
}

#endif