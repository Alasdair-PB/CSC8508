#include "TimerUI.h"
#include <filesystem>        
#include <string>
#include <functional>

using namespace NCL;
using namespace UI;

TimerUI::TimerUI() {
	std::function<CSC8508::PushdownState::PushdownResult()> func = [this]() -> CSC8508::PushdownState::PushdownResult {
		ImGui::Text(std::to_string(1.0f / dt).c_str());
		return CSC8508::PushdownState::PushdownResult::NoChange;
		};

	timerUI->PushVoidElement(func);
}

TimerUI::~TimerUI() {
	delete timerUI;
}