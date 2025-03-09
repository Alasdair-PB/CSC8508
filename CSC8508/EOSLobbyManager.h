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

private:
    EOSLobbyManager();
    ~EOSLobbyManager();

    static void OnLobbyCreated(const EOS_Lobby_CreateLobbyCallbackInfo* Data);
    static void OnLobbyUpdated(const EOS_Lobby_UpdateLobbyCallbackInfo* Data);

    EOS_HLobby LobbyHandle = nullptr;
};
