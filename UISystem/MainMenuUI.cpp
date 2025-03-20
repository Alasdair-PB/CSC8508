#include "MainMenuUI.h"
#include <filesystem>        

using namespace NCL;
using namespace UI;

MainMenuUI::MainMenuUI() {
	std::function<CSC8508::PushdownState::PushdownResult()> funcA = [this]() -> CSC8508::PushdownState::PushdownResult {
		menuOption = startOffline;
		return CSC8508::PushdownState::PushdownResult::NoChange;
	};

	std::function<CSC8508::PushdownState::PushdownResult()> funcB = [this]() -> CSC8508::PushdownState::PushdownResult {
		menuOption = startServer;
		return CSC8508::PushdownState::PushdownResult::NoChange;
	};

	std::function<CSC8508::PushdownState::PushdownResult()> funcC = [this]() -> CSC8508::PushdownState::PushdownResult {
		menuOption = startClient;
		return CSC8508::PushdownState::PushdownResult::NoChange;
	};

	menuUI->PushButtonElement(ImVec2(0.4f, 0.05f), "Start Offline", funcA);
	menuUI->PushButtonElement(ImVec2(0.4f, 0.05f), "Start Server", funcB);
	menuUI->PushButtonElement(ImVec2(0.4f, 0.05f), "Start Client", funcC);
}

MainMenuUI::~MainMenuUI() {
	delete menuUI;
}