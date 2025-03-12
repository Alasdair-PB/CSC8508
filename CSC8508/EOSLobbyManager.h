#ifndef EOS_LOBBY_MANAGER_H
#define EOS_LOBBY_MANAGER_H

#include <iostream>
#include <eos_sdk.h>
#include <eos_lobby.h>
#include <eos_auth.h>
#include "EOSInitialisationManager.h"

class EOSLobbyManager {
public:
    static EOSLobbyManager& GetInstance();
    EOS_HLobby GetLobbyHandle() const;
    void CreateLobby();
    EOS_HLobby LobbyHandle = nullptr;
    char LobbyId[256];
private:
    EOSLobbyManager();
    ~EOSLobbyManager();

    static void OnLobbyCreated(const EOS_Lobby_CreateLobbyCallbackInfo* Data);
    static void OnLobbyUpdated(const EOS_Lobby_UpdateLobbyCallbackInfo* Data);

    std::atomic<bool> lobbyCreated = false; // Track whether the lobby is created
};

#endif // EOS_LOBBY_MANAGER_H
