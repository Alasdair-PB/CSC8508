#include "GameVictoryUI.h"
#include <filesystem>        
#include <string>
#include <functional>

using namespace NCL;
using namespace UI;

GameVictoryUI::GameVictoryUI() {
	std::function<CSC8508::PushdownState::PushdownResult()> func = [this]() -> CSC8508::PushdownState::PushdownResult {
		ImGui::Text("Mission complete");
		return CSC8508::PushdownState::PushdownResult::NoChange;
		};

	std::function<CSC8508::PushdownState::PushdownResult()> quitFunc = [this]() -> CSC8508::PushdownState::PushdownResult {
		ImGui::Text("Press esc to exit");
		return CSC8508::PushdownState::PushdownResult::NoChange;
		};

	gameOverUI->PushVoidElement(func);
	gameOverUI->PushVoidElement(quitFunc);
}

GameVictoryUI::~GameVictoryUI() {
	delete gameOverUI;
}

void GameVictoryUI::PushElement(std::function<CSC8508::PushdownState::PushdownResult()> f) {
	gameOverUI->PushVoidElement(f);
}