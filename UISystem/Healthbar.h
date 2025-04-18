#pragma 
#include "UIElementsGroup.h"

namespace NCL {
	namespace UI {

		class Healthbar {

		public:
			Healthbar();
			~Healthbar();

			void UpdateHealth(int h) {
				health = h;
			}

			UIElementsGroup* healthbar = new UIElementsGroup(ImVec2(0.05f, 0.85f), ImVec2(0.4f, 0.4f), 1.0f, "Healthbar", 0.0f, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize);

		protected:
			int health = 100;
		};
	}
}