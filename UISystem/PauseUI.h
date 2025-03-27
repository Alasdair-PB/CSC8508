#pragma once
#include "UIElementsGroup.h"

namespace NCL {
	namespace UI {

		class PauseUI {
		public:
			PauseUI();
			~PauseUI();

			UIElementsGroup* pauseUI = new UIElementsGroup(ImVec2(0.5f, 0.5f), ImVec2(0.5f, 0.5f), 2.0f, "Pause", 0.0f, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar);

		protected:

		};
	}
}