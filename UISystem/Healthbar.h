#pragma once
#include "imgui.h"

namespace NCL {
	namespace UI {

		class Healthbar {

		public:
			Healthbar();
			~Healthbar();

			void Draw(int health);

		};
	}
}