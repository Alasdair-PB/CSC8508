#pragma once
#include "UIElementsGroup.h"

namespace NCL {
	namespace UI {

		class GameOverUI {
		public:
			GameOverUI();
			~GameOverUI();

			void PushElement(std::function<CSC8508::PushdownState::PushdownResult()> f);

			UIElementsGroup* gameOverUI = new UIElementsGroup(ImVec2(0.5f, 0.5f), ImVec2(0.5f, 0.5f), 2.0f, "Game Over", 0.0f, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar);

		protected:

		};
	}
}