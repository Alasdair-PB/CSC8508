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
#include <thread>

class EOSLobbyFunctions {
public:
    EOSLobbyFunctions(EOSInitialisationManager& initManager, EOSLobbySearch& lobbySearch);
    ~EOSLobbyFunctions();

    void JoinLobby();
    void LeaveLobby();
    static void OnLeaveLobbyComplete(const EOS_Lobby_LeaveLobbyCallbackInfo* Data);
    static void OnJoinLobbyComplete(const EOS_Lobby_JoinLobbyCallbackInfo* Data);
    void UpdateLobbyDetails();
    void RunUpdateLoop();
    std::string ownerIP; 
    std::string lobbyID;
    int playerCount = 0;
private:
    std::atomic<bool> running; // Flag to control the update loop
    EOSInitialisationManager& eosInitManager;
    EOSLobbySearch& eosSearchManager;


    bool hasStartedClientCallback = false; // Ensure it only runs once
};
