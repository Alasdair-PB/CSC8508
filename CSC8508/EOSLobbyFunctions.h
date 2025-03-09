#pragma once
#define EOS_LOBBY_FUNCTIONS_H

#include <iostream>
#include <vector>
#include <string>
#include "eos_sdk.h"
#include "eos_lobby.h"

class EOSLobbyFunctions {
public:
    static void JoinLobby();
    static void LeaveLobby();
    static void OnJoinLobbyComplete(const EOS_Lobby_JoinLobbyCallbackInfo* Data);
    static void OnLeaveLobbyComplete(const EOS_Lobby_LeaveLobbyCallbackInfo* Data);
    static std::vector<std::string> FetchLobbyMembers();
};
