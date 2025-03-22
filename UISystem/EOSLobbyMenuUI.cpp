#include "EOSLobbyMenuUI.h"
#include <filesystem>        

using namespace NCL;
using namespace UI;

// Needs to add EOS Menu too
EOSLobbyMenuUI::EOSLobbyMenuUI() {

	if (lobbyOwner)
	{
		std::function<CSC8508::PushdownState::PushdownResult()> funcA = [this]() -> CSC8508::PushdownState::PushdownResult {
			eosLobbyMenuOption = startGame;
			return CSC8508::PushdownState::PushdownResult::NoChange;
			};

		eosLobbyMenuUI->PushButtonElement(ImVec2(0.4f, 0.05f), "Host Lobby", funcA);
	}
}

EOSLobbyMenuUI::~EOSLobbyMenuUI() {
	delete eosLobbyMenuUI;
}

