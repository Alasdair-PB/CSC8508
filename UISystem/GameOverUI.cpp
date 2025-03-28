#include "GameOverUI.h"
#include <filesystem>        
#include <string>
#include <functional>

using namespace NCL;
using namespace UI;

GameOverUI::GameOverUI() {
	std::function<CSC8508::PushdownState::PushdownResult()> func = [this]() -> CSC8508::PushdownState::PushdownResult {
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.00f, 0.60f, 0.00f, 1.00f));
		ImGui::Text("Game Over");
		ImGui::PopStyleColor;
		return CSC8508::PushdownState::PushdownResult::NoChange;
		};

	std::function<CSC8508::PushdownState::PushdownResult()> quitFunc = [this]() -> CSC8508::PushdownState::PushdownResult {
		ImGui::Text("Press esc to exit");
		return CSC8508::PushdownState::PushdownResult::NoChange;
		};

	gameOverUI->PushVoidElement(func);
	gameOverUI->PushVoidElement(quitFunc);
}

GameOverUI::~GameOverUI() {
	delete gameOverUI;
}