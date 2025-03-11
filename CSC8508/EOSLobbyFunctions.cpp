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

    if (LocalUserId == nullptr) {
        std::cerr << "[ERROR] LocalUserId is null, cannot join lobby\n";
        return;
    }

    if (LobbyDetailsHandle == nullptr) {
        std::cerr << "[ERROR] Invalid LobbyDetailsHandle before JoinLobby()\n";
        return;
    }

    if (LobbyHandle == nullptr) {
        std::cerr << "[ERROR] Invalid LobbyHandle before JoinLobby()\n";
        return;
    }

    EOS_Lobby_JoinLobbyOptions JoinOptions = {};
    JoinOptions.ApiVersion = EOS_LOBBY_JOINLOBBY_API_LATEST;
    JoinOptions.LobbyDetailsHandle = LobbyDetailsHandle;
    JoinOptions.LocalUserId = LocalUserId;
    JoinOptions.LocalRTCOptions = NULL;
    JoinOptions.bPresenceEnabled = EOS_TRUE;
    JoinOptions.bCrossplayOptOut = EOS_FALSE;
    JoinOptions.RTCRoomJoinActionType = EOS_ELobbyRTCRoomJoinActionType::EOS_LRRJAT_AutomaticJoin;

    EOS_Lobby_JoinLobby(LobbyHandle, &JoinOptions, nullptr, EOSLobbyFunctions::OnJoinLobbyComplete);

    while (true) { // Add a flag to control the loop
        EOS_Platform_Tick(PlatformHandle);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }



}


void EOSLobbyFunctions::OnJoinLobbyComplete(const EOS_Lobby_JoinLobbyCallbackInfo* Data) {
    if (Data->ResultCode == EOS_EResult::EOS_Success) {
        std::cout << "[OnJoinLobbyComplete] Successfully joined lobby.\n";
        EOSLobbyFunctions& eosLobbyFunctions = EOSLobbyFunctions::GetInstance();

        // Call UpdateLobbyDetails on the instance
        eosLobbyFunctions.UpdateLobbyDetails();
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

    EOS_Lobby_LeaveLobby(LobbyHandle, &LeaveOptions, nullptr, OnLeaveLobbyComplete);
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

void EOSLobbyFunctions::UpdateLobbyDetails()
{
    while (true) {
        EOSLobbySearch& eosSearchManager = EOSLobbySearch::GetInstance();
        EOSInitialisationManager& eosInitManager = EOSInitialisationManager::GetInstance();
        EOS_HPlatform PlatformHandle = eosInitManager.GetPlatformHandle();
        EOS_HLobbyDetails LobbyDetailsHandle = eosSearchManager.GetLobbyDetailsHandle();

        if (!LobbyDetailsHandle)
        {
            std::cerr << "Error: Invalid lobby details handle." << std::endl;
            return;
        }

        // Clear the console screen
#ifdef _WIN32
        system("CLS");
#else
        system("clear");
#endif

        // Copy lobby details
        EOS_LobbyDetails_CopyInfoOptions copyInfoOptions = {};
        copyInfoOptions.ApiVersion = EOS_LOBBYDETAILS_COPYINFO_API_LATEST;

        EOS_LobbyDetails_Info* lobbyInfo = nullptr;
        EOS_EResult result = EOS_LobbyDetails_CopyInfo(LobbyDetailsHandle, &copyInfoOptions, &lobbyInfo);

        if (result != EOS_EResult::EOS_Success || !lobbyInfo)
        {
            std::cerr << "Error: Failed to copy lobby info." << std::endl;
            return;
        }

        // Display basic lobby data
        std::cout << "Lobby ID: " << lobbyInfo->LobbyId << std::endl;
        std::cout << "Max Members: " << lobbyInfo->MaxMembers << std::endl;

        char userIdString[EOS_PRODUCTUSERID_MAX_LENGTH + 1] = { 0 };
        int32_t bufferSize = sizeof(userIdString);

        if (EOS_ProductUserId_ToString(lobbyInfo->LobbyOwnerUserId, userIdString, &bufferSize) == EOS_EResult::EOS_Success) {
            std::cout << "Lobby owner: " << userIdString << std::endl;
        }
        else {
            std::cerr << "Failed to convert LobbyOwnerUserId to string!" << std::endl;
        }

        // Get local user ID
        EOS_ProductUserId localUserId = eosInitManager.GetLocalUserId();
        char localUserStr[EOS_PRODUCTUSERID_MAX_LENGTH] = {};

        if (EOS_ProductUserId_IsValid(localUserId))
        {
            EOS_EResult localToStringResult = EOS_ProductUserId_ToString(localUserId, localUserStr, &bufferSize);
            if (localToStringResult == EOS_EResult::EOS_Success)
            {
                std::cout << "You (Local User): " << localUserStr << std::endl;
            }
            else
            {
                std::cerr << "Error: Failed to convert Local User ID to string. Error code: " << static_cast<int>(localToStringResult) << std::endl;
            }
        }
        else
        {
            std::cerr << "Error: Invalid Local User ID." << std::endl;
        }

        // Get member count
        EOS_LobbyDetails_GetMemberCountOptions memberCountOptions = {};
        memberCountOptions.ApiVersion = EOS_LOBBYDETAILS_GETMEMBERCOUNT_API_LATEST;

        int32_t memberCount = EOS_LobbyDetails_GetMemberCount(LobbyDetailsHandle, &memberCountOptions);
        std::cout << "Other Members Count: " << (memberCount > 1 ? memberCount - 1 : 0) << std::endl;

        // Iterate through other members only if there are more than 1 members
        if (memberCount > 1)
        {
            // Start at index 1 if there are exactly 2 members (to skip the local user)
            int startIndex = (memberCount == 2) ? 1 : 0;

            for (int32_t i = startIndex; i < memberCount; ++i)
            {
                EOS_LobbyDetails_GetMemberByIndexOptions memberByIndexOptions = {};
                memberByIndexOptions.ApiVersion = EOS_LOBBYDETAILS_GETMEMBERBYINDEX_API_LATEST;
                memberByIndexOptions.MemberIndex = i;

                EOS_ProductUserId memberId = EOS_LobbyDetails_GetMemberByIndex(LobbyDetailsHandle, &memberByIndexOptions);
                char memberStr[EOS_PRODUCTUSERID_MAX_LENGTH] = {};
                bufferSize = sizeof(memberStr);

                if (!EOS_ProductUserId_IsValid(memberId))
                {
                    std::cerr << "Error: Invalid ProductUserId retrieved for member " << i + 1 << std::endl;
                    continue;  // Skip this iteration
                }

                EOS_EResult toStringResult = EOS_ProductUserId_ToString(memberId, memberStr, &bufferSize);
                if (toStringResult == EOS_EResult::EOS_Success)
                {
                    std::cout << "Member " << i << ": " << memberStr << std::endl;
                }
                else
                {
                    std::cerr << "Error: Failed to convert ProductUserId to string. Error code: " << static_cast<int>(toStringResult) << std::endl;
                }

                // If there are exactly 2 members, iterate only once
                if (memberCount == 2) break;
            }
        }

        // Tick the EOS platform and sleep for 5 seconds
        EOS_Platform_Tick(PlatformHandle);
        std::this_thread::sleep_for(std::chrono::seconds(5));

        eosSearchManager.CreateLobbySearch(lobbyInfo->LobbyId);
    }
}
