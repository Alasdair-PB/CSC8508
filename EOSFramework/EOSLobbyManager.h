#ifndef EOS_LOBBY_MANAGER_H
#define EOS_LOBBY_MANAGER_H

#if !PS5

#include <iostream>
#include <eos_sdk.h>
#include <eos_lobby.h>
#include <eos_auth.h>
#include "EOSInitialisationManager.h"

class EOSLobbyManager {
public:
    EOSLobbyManager(EOSInitialisationManager& eosManager);
    ~EOSLobbyManager();
    EOS_HLobby GetLobbyHandle() const;
    void CreateLobby();
    void RunUpdateLoop();
    EOS_HLobby LobbyHandle = nullptr;
    char LobbyId[256] = {};
private:
    bool running = false;

    static void OnLobbyCreated(const EOS_Lobby_CreateLobbyCallbackInfo* Data);
    static void OnLobbyUpdated(const EOS_Lobby_UpdateLobbyCallbackInfo* Data);

    std::atomic<bool> lobbyCreated = false;
    EOSInitialisationManager& eosManager;
};

#endif // !PS5

#endif // EOS_LOBBY_MANAGER_H
