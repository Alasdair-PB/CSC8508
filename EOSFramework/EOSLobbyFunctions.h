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
    void UpdateLobbyDetails();

    // Getter for the Owner IP
    std::string GetOwnerID() const { return ownerIP; }

private:
    std::string ownerIP;  // Store the Owner IP as a string
    bool hasStartedClientCallback = false; // Ensure it only runs once
};
