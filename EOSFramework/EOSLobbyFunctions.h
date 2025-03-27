#if EOSBUILD

#pragma once
#define EOS_LOBBY_FUNCTIONS_H

#include <iostream>
#include <vector>
#include <string>
#include <atomic>
#include <thread>
#include "eos_sdk.h"
#include "eos_lobby.h"
#include "EOSInitialisationManager.h"
#include "EOSLobbyManager.h"
#include "EOSLobbySearch.h"

class EOSLobbyFunctions {
public:
    EOSLobbyFunctions(EOSInitialisationManager& initManager, EOSLobbySearch& lobbySearch);
    ~EOSLobbyFunctions();

    void JoinLobby();
    void LeaveLobby();
    void UpdateLobbyDetails();
    void RunUpdateLoop();

    static void OnLeaveLobbyComplete(const EOS_Lobby_LeaveLobbyCallbackInfo* Data);
    static void OnJoinLobbyComplete(const EOS_Lobby_JoinLobbyCallbackInfo* Data);

    std::string ownerIP;
    std::string lobbyID;
    int playerCount = 0;

private:
    std::atomic<bool> running;
    EOSInitialisationManager& eosInitManager;
    EOSLobbySearch& eosSearchManager;
    bool hasStartedClientCallback = false;

    bool ValidateLobbyDetailsHandle(EOS_HLobbyDetails handle);
    EOS_LobbyDetails_Info* CopyLobbyInfo(EOS_HLobbyDetails handle);
    bool DetermineIfOwner(EOS_ProductUserId ownerId);
    std::string GetOwnerIPAttribute(EOS_HLobbyDetails handle);
    void ParseOwnerIP(const std::string& ip);
    std::vector<EOS_ProductUserId> GetLobbyMembers(EOS_HLobbyDetails handle);
};

#endif