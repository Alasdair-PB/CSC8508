#pragma once
#include "imgui.h"

namespace NCL {
	namespace UI {

		class FramerateUI {
		public:
			FramerateUI();
			~FramerateUI();

			void Draw(float dt, int winWidth, int winHeight);

		protected:
			float dt = 0;
		};
	}
}