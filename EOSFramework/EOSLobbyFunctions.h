#pragma once
#define EOS_LOBBY_FUNCTIONS_H

#include <iostream>
#include <vector>
#include <string>
#include "eos_sdk.h"
#include "eos_lobby.h"
#include "EOSInitialisationManager.h"
#include "EOSLobbyManager.h"
#include "EOSLobbySearch.h"

class EOSLobbyFunctions {
public:
    EOSLobbyFunctions(EOSInitialisationManager& initManager, EOSLobbyManager& lobbyManager, EOSLobbySearch& lobbySearch)
        : eosInitManager(initManager), eosManager(lobbyManager), eosSearchManager(lobbySearch) {
    }

    void JoinLobby();
    void LeaveLobby();
    static void OnLeaveLobbyComplete(const EOS_Lobby_LeaveLobbyCallbackInfo* Data);
    static void OnJoinLobbyComplete(const EOS_Lobby_JoinLobbyCallbackInfo* Data);
    void UpdateLobbyDetails();

    std::string GetOwnerID() const { return ownerIP; }

private:
    EOSInitialisationManager& eosInitManager;
    EOSLobbyManager& eosManager;
    EOSLobbySearch& eosSearchManager;

    std::string ownerIP;  // Store the Owner IP as a string
    bool hasStartedClientCallback = false; // Ensure it only runs once
};
