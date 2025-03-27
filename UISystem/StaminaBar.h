#pragma once
#include "UIElementsGroup.h"

namespace NCL {
	namespace UI {

		class StaminaBar {

		public:
			StaminaBar();
			~StaminaBar();

			void UpdateStamina(int h) {
				stamina = h;
			}

			UIElementsGroup* staminaBar = new UIElementsGroup(ImVec2(0.05f, 0.9f), ImVec2(0.4, 0.4), 1.0f, "Stamina Bar", 0.0f, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize);

		protected:
			int stamina = 100;
		};
	}
}