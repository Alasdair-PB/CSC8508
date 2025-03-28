#pragma once
#include "UIElementsGroup.h"

namespace NCL {
	namespace UI {

		class TimerUI {
		public:
			TimerUI();
			~TimerUI();

			void UpdateTimer(float t) { time = t; }

			UIElementsGroup* timerUI = new UIElementsGroup(ImVec2(0.9f, 0.05f), ImVec2(0.08f, 0.08f), 1.0f, "Timer", 0.0f, ImGuiWindowFlags_NoResize);

		protected:
			float time = 0;
		};
	}
}