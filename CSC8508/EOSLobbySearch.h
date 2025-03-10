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

class EOSLobbySearch {
public:
    static EOSLobbySearch& GetInstance();

    void CreateLobbySearch(const char* TargetLobbyId);
    EOS_HLobbySearch GetLobbySearchHandle() const;
    EOS_HLobbyDetails GetLobbyDetailsHandle() const;
    std::vector<std::string> GetLobbyMemberIds() const; // Function to retrieve user IDs
    static void OnFindLobbiesComplete(const EOS_LobbySearch_FindCallbackInfo* Data);

private:
    EOSLobbySearch();
    ~EOSLobbySearch();

    EOS_HLobbySearch LobbySearchHandle = nullptr;
    EOS_HLobbyDetails LobbyDetailsHandle;
    std::vector<std::string> LobbyMemberIds; // List to store user IDs
};