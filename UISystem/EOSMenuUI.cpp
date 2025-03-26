#if !PS5
#include "EOSMenuUI.h"
#include <filesystem>        

using namespace NCL;
using namespace UI;

// Needs to add EOS Menu too
EOSMenuUI::EOSMenuUI() {
	std::function<CSC8508::PushdownState::PushdownResult()> funcA = [this]() -> CSC8508::PushdownState::PushdownResult {
		menuOption = hostLobby;
		return CSC8508::PushdownState::PushdownResult::NoChange;
		};

	std::function<CSC8508::PushdownState::PushdownResult()> funcB = [this]() -> CSC8508::PushdownState::PushdownResult {
		menuOption = joinLobby;
		return CSC8508::PushdownState::PushdownResult::NoChange;
		};

	eosMenuUI->PushButtonElement(ImVec2(0.4f, 0.05f), "Host Lobby", funcA);
	eosMenuUI->PushButtonElement(ImVec2(0.4f, 0.05f), "Join Lobby", funcB);
}

EOSMenuUI::~EOSMenuUI() {
	delete eosMenuUI;
}

#endif