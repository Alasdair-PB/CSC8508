#include "EOSLobbyMenuUI.h"
#include <filesystem>        

using namespace NCL;
using namespace UI;

// Needs to add EOS Menu too
EOSLobbyMenuUI::EOSLobbyMenuUI(bool lobbyOwner) {
	if (lobbyOwner)
	{
		std::cout << "Lobby Owner in IF";
		std::function<CSC8508::PushdownState::PushdownResult()> funcA = [this]() -> CSC8508::PushdownState::PushdownResult {
			eosLobbyMenuOption = startGame;
			return CSC8508::PushdownState::PushdownResult::NoChange;
			};

		eosLobbyMenuUI->PushButtonElement(ImVec2(0.4f, 0.05f), "Start Game", funcA);
	}
}

EOSLobbyMenuUI::~EOSLobbyMenuUI() {
	delete eosLobbyMenuUI;
}

