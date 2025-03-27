#pragma once
#include "imgui.h"
#include "UIElementsGroup.h"

namespace NCL {
	namespace UI {

		class TimerUI {
		public:
			TimerUI();
			~TimerUI();

			void UpdateTimer(float delta) { dt = delta; }

			UIElementsGroup* timerUI = new UIElementsGroup(ImVec2(0.95f, 0.05f), ImVec2(0.08f, 0.08f), 1.0f, "Timer", 0.0f, ImGuiWindowFlags_NoResize);

		protected:
			float dt = 60;
		};
	}
}