#pragma once
#include "imgui.h"
#include "UIElementsGroup.h"

namespace NCL {
	namespace UI {

		class FramerateUI {
		public:
			FramerateUI();
			~FramerateUI();

			void UpdateFramerate(float delta) { dt = delta; }

			UIElementsGroup* frameUI = new UIElementsGroup(ImVec2(50, 50), ImVec2(120, 50), 1.0f, "Framerate");

		protected:
			float dt = 60;
		};
	}
}