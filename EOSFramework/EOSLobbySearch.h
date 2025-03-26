#if !PS5
#pragma once
#define EOS_LOBBY_SEARCH_H

#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <string>
#include "eos_sdk.h"
#include "eos_lobby.h"
#include "EOSInitialisationManager.h"
#include "EOSLobbyManager.h"

class EOSLobbySearch {
public:
    EOSLobbySearch(EOSInitialisationManager& initManager);
    ~EOSLobbySearch();

    void CreateLobbySearch(const char* TargetLobbyId);
    EOS_HLobbySearch GetLobbySearchHandle() const;
    EOS_HLobbyDetails GetLobbyDetailsHandle() const;
    std::vector<std::string> GetLobbyMemberIds() const;
    static void OnFindLobbiesComplete(const EOS_LobbySearch_FindCallbackInfo* Data);
    EOS_HLobbyDetails LobbyDetailsHandle = nullptr;
    EOS_HLobby LobbyHandle = nullptr;

    void RunUpdateLoop();

private:
    std::atomic<bool> running; // Flag to control the update loop
    EOS_HLobbySearch LobbySearchHandle = nullptr;
    
    std::vector<std::string> LobbyMemberIds;

    EOSInitialisationManager& eosInitManager;
};
#endif