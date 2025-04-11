#pragma once
#include "UIElementsGroup.h"

namespace NCL {
	namespace UI {

		class StaminaBar {

		public:
			StaminaBar();
			~StaminaBar();

			void UpdateStamina(float h) {
				stamina = h;
			}

			UIElementsGroup* staminaBar = new UIElementsGroup(ImVec2(0.05f, 0.9f), ImVec2(0.4f, 0.4f), 1.0f, "Stamina Bar", 0.0f, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize);

		protected:
			float stamina = 100;
		};
	}
}