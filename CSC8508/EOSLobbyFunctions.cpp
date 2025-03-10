#include "EOSLobbyFunctions.h"
#include "EOSInitialisationManager.h"
#include "EOSLobbyManager.h"
#include "EOSLobbySearch.h"

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

std::vector<std::string> EOSLobbyFunctions::FetchLobbyMembers() {
    std::vector<std::string> memberIds;

    EOSInitialisationManager& eosInitManager = EOSInitialisationManager::GetInstance();
    EOS_HPlatform PlatformHandle = eosInitManager.GetPlatformHandle();

    EOSLobbySearch& eosSearchManager = EOSLobbySearch::GetInstance();
    EOS_HLobbyDetails LobbyDetailsHandle = eosSearchManager.GetLobbyDetailsHandle();

    if (LobbyDetailsHandle == nullptr) {
        std::cerr << "[ERROR] LobbyDetailsHandle is null, cannot fetch members.\n";
        return memberIds;
    }

    // Initialize options for getting the member count
    EOS_LobbyDetails_GetMemberCountOptions MemberCountOptions = {};
    MemberCountOptions.ApiVersion = EOS_LOBBYDETAILS_GETMEMBERCOUNT_API_LATEST;

    // Get the member count using the correct options structure
    uint32_t MemberCount = EOS_LobbyDetails_GetMemberCount(LobbyDetailsHandle, &MemberCountOptions);
    std::cout << "[FetchLobbyMembers] Total members in lobby: " << MemberCount << "\n";

    if (MemberCount == 0) {
        std::cerr << "[WARNING] No members found in the lobby.\n";
        return memberIds;
    }

    // Reserve space in vector to avoid multiple allocations
    memberIds.reserve(MemberCount);

    // Iterate through the lobby members
    for (uint32_t i = 0; i < MemberCount; ++i) {
        EOS_LobbyDetails_GetMemberByIndexOptions GetMemberOptions = {};
        GetMemberOptions.ApiVersion = EOS_LOBBYDETAILS_GETMEMBERBYINDEX_API_LATEST;
        GetMemberOptions.MemberIndex = i;

        EOS_ProductUserId MemberId = EOS_LobbyDetails_GetMemberByIndex(LobbyDetailsHandle, &GetMemberOptions);
        if (MemberId) {
            char MemberIdString[EOS_PRODUCTUSERID_MAX_LENGTH];
            int32_t BufferSize = sizeof(MemberIdString);
            EOS_ProductUserId_ToString(MemberId, MemberIdString, &BufferSize);

            memberIds.push_back(std::string(MemberIdString));
        }
        else {
            std::cerr << "[ERROR] Failed to retrieve member ID at index " << i << "\n";
        }
    }

    return memberIds;
}
