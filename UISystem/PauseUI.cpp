#include "PauseUI.h"
#include <filesystem>        
#include <string>
#include <functional>

using namespace NCL;
using namespace UI;

PauseUI::PauseUI() {
	std::function<CSC8508::PushdownState::PushdownResult()> func = [this]() -> CSC8508::PushdownState::PushdownResult {
		ImGui::Text("Paused");
		return CSC8508::PushdownState::PushdownResult::NoChange;
		};

	pauseUI->PushVoidElement(func);
}

PauseUI::~PauseUI() {
	delete pauseUI;
}
