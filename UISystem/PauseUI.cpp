#include "PauseUI.h"
#include <filesystem>        
#include <string>
#include <functional>

using namespace NCL;
using namespace UI;

PauseUI::PauseUI() {
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
	std::function<CSC8508::PushdownState::PushdownResult()> func = [this]() -> CSC8508::PushdownState::PushdownResult {
		ImGui::Text("Paused");
		return CSC8508::PushdownState::PushdownResult::NoChange;
		};

	std::function<CSC8508::PushdownState::PushdownResult()> quitFunc = [this]() -> CSC8508::PushdownState::PushdownResult {
		ImGui::Text("Press esc to quit");
		return CSC8508::PushdownState::PushdownResult::NoChange;
		};

	pauseUI->PushVoidElement(func);
	pauseUI->PushVoidElement(quitFunc);
}

PauseUI::~PauseUI() {
	delete pauseUI;
}

void PauseUI::PushButtonElement(std::function<CSC8508::PushdownState::PushdownResult()> f, std::string name) {
	pauseUI->PushButtonElement(ImVec2(0.1, 0.1), name, f);
}

