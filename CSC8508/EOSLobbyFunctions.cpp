#include "EOSLobbyFunctions.h"
#include "EOSInitialisationManager.h"
#include "EOSLobbyManager.h"
#include "EOSLobbySearch.h"


// Enables the user to join a lobby with a valid lobby handle specified in the lobby search function
void EOSLobbyFunctions::JoinLobby() {
    EOSInitialisationManager& eosInitManager = EOSInitialisationManager::GetInstance();
    EOS_ProductUserId LocalUserId = eosInitManager.GetLocalUserId();
    EOS_HPlatform PlatformHandle = eosInitManager.GetPlatformHandle();

    EOSLobbyManager& eosManager = EOSLobbyManager::GetInstance();
    EOS_HLobby LobbyHandle = eosManager.GetLobbyHandle();

    EOSLobbySearch& eosSearchManager = EOSLobbySearch::GetInstance();
    EOS_HLobbyDetails LobbyDetailsHandle = eosSearchManager.GetLobbyDetailsHandle();

    if (LobbyDetailsHandle == nullptr) {
        std::cerr << "[ERROR] Invalid LobbyDetailsHandle before JoinLobby()\n";
        return;
    }

    if (LocalUserId == nullptr) {
        std::cerr << "[ERROR] LocalUserId is null, cannot join lobby\n";
        return;
    }

    LobbyHandle = EOS_Platform_GetLobbyInterface(PlatformHandle);

    EOS_Lobby_JoinLobbyOptions JoinOptions = {};
    JoinOptions.ApiVersion = EOS_LOBBY_JOINLOBBY_API_LATEST;
    JoinOptions.LobbyDetailsHandle = LobbyDetailsHandle;
    JoinOptions.LocalUserId = LocalUserId;
    JoinOptions.LocalRTCOptions = NULL;
    JoinOptions.bPresenceEnabled = EOS_TRUE;
    JoinOptions.bCrossplayOptOut = EOS_FALSE;
    JoinOptions.RTCRoomJoinActionType = EOS_ELobbyRTCRoomJoinActionType::EOS_LRRJAT_AutomaticJoin;

    EOS_Lobby_JoinLobby(LobbyHandle, &JoinOptions, nullptr, EOSLobbyFunctions::OnJoinLobbyComplete);
}


void EOSLobbyFunctions::OnJoinLobbyComplete(const EOS_Lobby_JoinLobbyCallbackInfo* Data) {
    if (Data->ResultCode == EOS_EResult::EOS_Success) {
        std::cout << "[OnJoinLobbyComplete] Successfully joined lobby.\n";
    }
    else {
        std::cerr << "[ERROR] Failed to join lobby. Error: " << EOS_EResult_ToString(Data->ResultCode) << std::endl;
    }
}

void EOSLobbyFunctions::OnLobbyUpdated(const EOS_Lobby_UpdateLobbyCallbackInfo* Data) {
    if (Data->ResultCode == EOS_EResult::EOS_Success) {
        std::cout << "[OnLobbyUpdated] Successfully updated lobby attributes.\n";
    }
    else {
        std::cerr << "[ERROR] Failed to update lobby. Error: " << EOS_EResult_ToString(Data->ResultCode) << std::endl;
    }
}

void EOSLobbyFunctions::LeaveLobby() {
    EOSInitialisationManager& eosInitManager = EOSInitialisationManager::GetInstance();
    EOS_ProductUserId LocalUserId = eosInitManager.GetLocalUserId();
    EOS_HPlatform PlatformHandle = eosInitManager.GetPlatformHandle();

    EOSLobbyManager& eosManager = EOSLobbyManager::GetInstance();
    EOS_HLobby LobbyHandle = eosManager.GetLobbyHandle();

    EOSLobbySearch& eosSearchManager = EOSLobbySearch::GetInstance();
    EOS_HLobbyDetails LobbyDetailsHandle = eosSearchManager.GetLobbyDetailsHandle();
    LobbyHandle = EOS_Platform_GetLobbyInterface(PlatformHandle);

    EOS_Lobby_LeaveLobbyOptions LeaveOptions = {};
    LeaveOptions.ApiVersion = EOS_LOBBY_LEAVELOBBY_API_LATEST;
    LeaveOptions.LocalUserId = LocalUserId;
    LeaveOptions.LobbyId = "8b04b581a080487c98ee5ee27338e65b";

    EOS_Lobby_LeaveLobby(LobbyHandle, &LeaveOptions, nullptr, EOSLobbyFunctions::OnLeaveLobbyComplete);
}

void EOSLobbyFunctions::OnLeaveLobbyComplete(const EOS_Lobby_LeaveLobbyCallbackInfo* Data) {
    if (Data->ResultCode == EOS_EResult::EOS_Success) {
        std::cout << "[OnLeaveLobbyComplete] Successfully left lobby: " << Data->LobbyId << "\n";
    }
    else {
        std::cerr << "[ERROR] Failed to leave lobby. Error: " << EOS_EResult_ToString(Data->ResultCode) << std::endl;
    }
}