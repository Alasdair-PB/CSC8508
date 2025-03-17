#pragma once
#include "imgui.h"

namespace NCL {
	namespace UI {

		class FramerateUI {
		public:
			FramerateUI();
			~FramerateUI();

			void Draw(float dt);

		protected:
			float dt = 0;
		};
	}
}