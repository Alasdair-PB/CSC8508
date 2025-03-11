#pragma once
#include "imgui.h"
#include "Healthbar.h"

namespace NCL {
	namespace CSC8508 {

		class Healthbar {

		public:
			Healthbar();
			~Healthbar();

			void Draw(int health);

		};
	}
}