#pragma once
#include "imgui.h"
#include "Healthbar.h"

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