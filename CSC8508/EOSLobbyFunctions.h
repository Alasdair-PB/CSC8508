#pragma once
#define EOS_LOBBY_FUNCTIONS_H

#include <iostream>
#include <vector>
#include <string>
#include "eos_sdk.h"
#include "eos_lobby.h"

class EOSLobbyFunctions {
public:
    static EOSLobbyFunctions& GetInstance() {
        static EOSLobbyFunctions instance; // Static instance of EOSLobbyFunctions
        return instance;
    }

    static void JoinLobby();
    static void LeaveLobby();
    static void OnLeaveLobbyComplete(const EOS_Lobby_LeaveLobbyCallbackInfo* Data);
    static void OnJoinLobbyComplete(const EOS_Lobby_JoinLobbyCallbackInfo* Data);
    static std::vector<std::string> FetchLobbyMembers();
    void UpdateLobbyDetails();
};
