#pragma once
#include "imgui.h"
#include "UIElementsGroup.h"

namespace NCL {
	namespace UI {

		class FramerateUI {
		public:
			FramerateUI();
			~FramerateUI();

			CSC8508::PushdownState::PushdownResult OnElementsRender(float dt);

			/*void Draw(float dt, int winWidth, int winHeight);*/

		protected:
			float dt = 0;
		};
	}
}