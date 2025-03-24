#include "EOSLobbyMenuUI.h"
#include <filesystem>        

using namespace NCL;
using namespace UI;

EOSLobbyMenuUI::EOSLobbyMenuUI(bool lobbyOwner, const std::string& ip, const std::string& lobbyID, int playerCount) {
	if (lobbyOwner)
	{
		std::function<CSC8508::PushdownState::PushdownResult()> funcA = [this]() -> CSC8508::PushdownState::PushdownResult {
			menuOption = startGameAsHost;
			std::cout << "[EOSLobbyMenuUI.cpp] Button Press Received";
			return CSC8508::PushdownState::PushdownResult::NoChange;
			};

		eosLobbyMenuUI->PushButtonElement(ImVec2(0.4f, 0.05f), "Start Game", funcA);
	}
	else
	{
		std::function<CSC8508::PushdownState::PushdownResult()> funcB = [this]() -> CSC8508::PushdownState::PushdownResult {
			menuOption = startGameAsJoin;
			std::cout << "[EOSLobbyMenuUI.cpp] Button Press Received";
			return CSC8508::PushdownState::PushdownResult::NoChange;
			};

		eosLobbyMenuUI->PushButtonElement(ImVec2(0.4f, 0.05f), "Start Game", funcB);
	}

	// Display IP as static label
	std::string ipLabel = "IP: " + ip;
	eosLobbyMenuUI->PushButtonElement(ImVec2(0.4f, 0.05f), ipLabel, nullptr);

	// Display Lobby ID as static label
	std::string idLabel = "Lobby ID: " + lobbyID;
	eosLobbyMenuUI->PushButtonElement(ImVec2(0.4f, 0.05f), idLabel, nullptr);

	// Display Player Count as static label
	std::string playersLabel = "Players: " + std::to_string(playerCount);
	eosLobbyMenuUI->PushButtonElement(ImVec2(0.4f, 0.05f), playersLabel, nullptr);
}


EOSLobbyMenuUI::~EOSLobbyMenuUI() {
	delete eosLobbyMenuUI;
}

