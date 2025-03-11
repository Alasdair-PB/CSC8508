#pragma once
#include "imgui.h"

namespace NCL {
	namespace CSC8508 {

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