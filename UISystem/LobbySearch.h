#pragma once
#include "UIElementsGroup.h"

namespace NCL {
	namespace UI {
		class LobbySearch {
		public:
			LobbySearch();
			~LobbySearch();

			UIElementsGroup* lobbySearchField = new UIElementsGroup(ImVec2(0.4f, 0.1f), ImVec2(0.2f, 0.05f), 1.0f, "Lobby Search Field", 0.0f,
				ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar);

		protected:
			std::string inputText;
		};
	}
}