#include "LobbySearch.h"
#include <filesystem>        
#include <iostream>

using namespace NCL;
using namespace UI;

LobbySearch::LobbySearch() {
	std::function<CSC8508::PushdownState::PushdownResult(std::string)> func = [this](std::string input) -> CSC8508::PushdownState::PushdownResult {
		inputText = input;
		return CSC8508::PushdownState::PushdownResult::NoChange;
		};

	lobbySearchField->PushInputFieldElement("Lobby", inputText, func);



}

LobbySearch::~LobbySearch() {
	delete lobbySearchField;
}