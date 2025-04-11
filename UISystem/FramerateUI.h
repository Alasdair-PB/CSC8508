#pragma once
#include "UIElementsGroup.h"

namespace NCL {
	namespace UI {

		class FramerateUI {
		public:
			FramerateUI();
			~FramerateUI();

			void UpdateFramerate(float delta) { dt = delta; }

			UIElementsGroup* frameUI = new UIElementsGroup(ImVec2(0.05f, 0.05f), ImVec2(0.08f, 0.08f), 1.0f, "Framerate", 0.0f, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoNav);

		protected:
			float dt = 0.017f;
		};
	}
} 