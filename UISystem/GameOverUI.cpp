#include "GameOverUI.h"
#include <filesystem>        
#include <string>
#include <functional>

using namespace NCL;
using namespace UI;

GameOverUI::GameOverUI() {
	std::function<CSC8508::PushdownState::PushdownResult()> func = [this]() -> CSC8508::PushdownState::PushdownResult {
		ImGui::Text("Game Over");
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

void GameOverUI::PushElement(std::function<CSC8508::PushdownState::PushdownResult()> f) {
	gameOverUI->PushVoidElement(f);
}