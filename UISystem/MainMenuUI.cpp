#include "MainMenuUI.h"
#include <filesystem>        

using namespace NCL;
using namespace UI;

// Needs to add EOS Menu too
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

#if EOSBUILD

	std::function<CSC8508::PushdownState::PushdownResult()> funcD = [this]() -> CSC8508::PushdownState::PushdownResult {
		menuOption = startEOS;
		return CSC8508::PushdownState::PushdownResult::NoChange;
		};

	menuUI->PushButtonElement(ImVec2(0.4f, 0.05f), "StartEOS", funcD);

#endif

	menuUI->PushButtonElement(ImVec2(0.4f, 0.05f), "Start Offline", funcA);
	menuUI->PushButtonElement(ImVec2(0.4f, 0.05f), "Start Server", funcB);
	menuUI->PushButtonElement(ImVec2(0.4f, 0.05f), "Start Client", funcC);
	
}

MainMenuUI::~MainMenuUI() {
	delete menuUI;
}

