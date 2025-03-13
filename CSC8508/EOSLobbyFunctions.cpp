#include "EOSLobbyFunctions.h"
#include "EOSInitialisationManager.h"
#include "EOSLobbyManager.h"
#include "EOSLobbySearch.h"
#include <unordered_map>
#include "NetworkedGame.h"

// Attempts to join a lobby
void EOSLobbyFunctions::JoinLobby() {
    // Retrieves the local user's EOS ProductUserId and platform handle from the initialisation manager
    EOSInitialisationManager& eosInitManager = EOSInitialisationManager::GetInstance();
    EOS_ProductUserId LocalUserId = eosInitManager.GetLocalUserId();
    EOS_HPlatform PlatformHandle = eosInitManager.GetPlatformHandle();

    // Gets the EOS lobby handle from the lobby manager
    EOSLobbyManager& eosManager = EOSLobbyManager::GetInstance();
    EOS_HLobby LobbyHandle = eosManager.GetLobbyHandle();

    // Gets the handle for the specific lobby details to join
    EOSLobbySearch& eosSearchManager = EOSLobbySearch::GetInstance();
    EOS_HLobbyDetails LobbyDetailsHandle = eosSearchManager.GetLobbyDetailsHandle();

    // Ensures that the LocalUserId is valid before attempting to join the lobby
    if (LocalUserId == nullptr) {
        std::cerr << "[ERROR] LocalUserId is null, cannot join lobby\n";
        return;
    }

    // Ensures that a valid lobby details handle is available
    if (LobbyDetailsHandle == nullptr) {
        std::cerr << "[ERROR] Invalid LobbyDetailsHandle before JoinLobby()\n";
        return;
    }

    // Ensures that a valid lobby handle is available
    if (LobbyHandle == nullptr) {
        std::cerr << "[ERROR] Invalid LobbyHandle before JoinLobby()\n";
        return;
    }

    // Configures the options for joining a lobby
    EOS_Lobby_JoinLobbyOptions JoinOptions = {};
    JoinOptions.ApiVersion = EOS_LOBBY_JOINLOBBY_API_LATEST;
    JoinOptions.LobbyDetailsHandle = LobbyDetailsHandle;
    JoinOptions.LocalUserId = LocalUserId;
    JoinOptions.LocalRTCOptions = NULL;
    JoinOptions.bPresenceEnabled = EOS_TRUE;
    JoinOptions.bCrossplayOptOut = EOS_FALSE;
    JoinOptions.RTCRoomJoinActionType = EOS_ELobbyRTCRoomJoinActionType::EOS_LRRJAT_AutomaticJoin;

    // Initiates the lobby join request
    EOS_Lobby_JoinLobby(LobbyHandle, &JoinOptions, nullptr, EOSLobbyFunctions::OnJoinLobbyComplete);

    // Loop to continuously update the EOS platform services
    while (true) { // Add a flag to control the loop
        EOS_Platform_Tick(PlatformHandle);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

// Callback function triggered when the attempt to join a lobby completes
void EOSLobbyFunctions::OnJoinLobbyComplete(const EOS_Lobby_JoinLobbyCallbackInfo* Data) {
    if (Data->ResultCode == EOS_EResult::EOS_Success) {
        std::cout << "[OnJoinLobbyComplete] Successfully joined lobby.\n";
        EOSLobbyFunctions& eosLobbyFunctions = EOSLobbyFunctions::GetInstance();

        // Calls UpdateLobbyDetails to refresh lobby information after joining
        eosLobbyFunctions.UpdateLobbyDetails();
    }
    else {
        std::cerr << "[ERROR] Failed to join lobby. Error: " << EOS_EResult_ToString(Data->ResultCode) << std::endl;
    }
}

// Attempts to leave a lobby
void EOSLobbyFunctions::LeaveLobby() {
    // Retrieves the local user's local user ID and platform handle from the initialisation manager
    EOSInitialisationManager& eosInitManager = EOSInitialisationManager::GetInstance();
    EOS_ProductUserId LocalUserId = eosInitManager.GetLocalUserId();
    EOS_HPlatform PlatformHandle = eosInitManager.GetPlatformHandle();

    // Gets the lobby handle from the lobby manager
    EOSLobbyManager& eosManager = EOSLobbyManager::GetInstance();
    EOS_HLobby LobbyHandle = eosManager.GetLobbyHandle();

    // Gets the handle for the specific lobby details from the search manager
    EOSLobbySearch& eosSearchManager = EOSLobbySearch::GetInstance();
    LobbyHandle = EOS_Platform_GetLobbyInterface(PlatformHandle);


    // Configures the options for leaving a lobby
    EOS_Lobby_LeaveLobbyOptions LeaveOptions = {};
    LeaveOptions.ApiVersion = EOS_LOBBY_LEAVELOBBY_API_LATEST;
    LeaveOptions.LocalUserId = LocalUserId;
    LeaveOptions.LobbyId = "8b04b581a080487c98ee5ee27338e65b";

    // Initiates the request to leave the lobby
    EOS_Lobby_LeaveLobby(LobbyHandle, &LeaveOptions, nullptr, OnLeaveLobbyComplete);
}

// Callback function triggered when the attempt to leave a lobby completes
void EOSLobbyFunctions::OnLeaveLobbyComplete(const EOS_Lobby_LeaveLobbyCallbackInfo* Data) {
    if (Data->ResultCode == EOS_EResult::EOS_Success) {
        std::cout << "[OnLeaveLobbyComplete] Successfully left lobby: " << Data->LobbyId << "\n";
    }
    else {
        std::cerr << "[ERROR] Failed to leave lobby. Error: " << EOS_EResult_ToString(Data->ResultCode) << std::endl;
    }
}

// Continuously updates and synchronises lobby details with all members

/*
 * Once the user receives the owner's ip, they should send their own ip across to the owner
*/

void EOSLobbyFunctions::UpdateLobbyDetails()
{
    // Stores users EOS IDs along with their IP addresses
    std::unordered_map<std::string, std::string> collectedIPs; 

    // Retrieves instances of required EOS managers
    EOSLobbySearch& eosSearchManager = EOSLobbySearch::GetInstance();
    EOSInitialisationManager& eosInitManager = EOSInitialisationManager::GetInstance();
    EOS_HPlatform PlatformHandle = eosInitManager.GetPlatformHandle();

    

    // This loop continuously updates lobby information
    while (true) {
        EOS_HLobbyDetails LobbyDetailsHandle = eosSearchManager.GetLobbyDetailsHandle();

        // Ensures a valid lobby details handle is available
        if (!LobbyDetailsHandle)
        {
            std::cerr << "Error: Invalid lobby details handle." << std::endl;
            return;
        }

        // Clears the console screen
        system("CLS");

        // Retrieves lobby information
        EOS_LobbyDetails_CopyInfoOptions copyInfoOptions = {};
        copyInfoOptions.ApiVersion = EOS_LOBBYDETAILS_COPYINFO_API_LATEST;

        EOS_LobbyDetails_Info* lobbyInfo = nullptr;
        EOS_EResult result = EOS_LobbyDetails_CopyInfo(LobbyDetailsHandle, &copyInfoOptions, &lobbyInfo);

        if (result != EOS_EResult::EOS_Success || !lobbyInfo)
        {
            std::cerr << "Error: Failed to copy lobby info." << std::endl;
            return;
        }

        std::cout << "Lobby ID: " << lobbyInfo->LobbyId << std::endl;
        std::cout << "Max Members: " << lobbyInfo->MaxMembers << std::endl;

        // Retrieves the lobby owner ID as a string
        char ownerUserIdString[EOS_PRODUCTUSERID_MAX_LENGTH + 1] = { 0 };
        int32_t bufferSize = sizeof(ownerUserIdString);
        bool isOwner = false;

        if (EOS_ProductUserId_ToString(lobbyInfo->LobbyOwnerUserId, ownerUserIdString, &bufferSize) == EOS_EResult::EOS_Success) {
            std::cout << "Lobby owner: " << ownerUserIdString << std::endl;
        }
        else {
            std::cerr << "Failed to convert LobbyOwnerUserId to string!" << std::endl;
        }

        // Retrieves the local user ID
        EOS_ProductUserId localUserId = eosInitManager.GetLocalUserId();
        char localUserStr[EOS_PRODUCTUSERID_MAX_LENGTH + 1] = {};
        bufferSize = EOS_PRODUCTUSERID_MAX_LENGTH + 1;

        if (EOS_ProductUserId_IsValid(localUserId))
        {
            EOS_EResult localToStringResult = EOS_ProductUserId_ToString(localUserId, localUserStr, &bufferSize);
            if (localToStringResult == EOS_EResult::EOS_Success)
            {
                std::cout << "You (Local User): " << localUserStr << std::endl;

                // Determines if the local user is the owner
                if (strcmp(localUserStr, ownerUserIdString) == 0) {
                    isOwner = true;
                }
            }
            else
            {
                std::cerr << "Error: Failed to convert Local User ID to string. Error code: "
                    << static_cast<int>(localToStringResult) << std::endl;
            }
        }
        else
        {
            std::cerr << "Error: Invalid Local User ID. Check EOS authentication." << std::endl;
        }

        // Retrieve the number of attributes
        EOS_LobbyDetails_GetAttributeCountOptions attributeCountOptions = {};
        attributeCountOptions.ApiVersion = EOS_LOBBYDETAILS_GETATTRIBUTECOUNT_API_LATEST;
        int32_t attributeCount = EOS_LobbyDetails_GetAttributeCount(LobbyDetailsHandle, &attributeCountOptions);

        // Ensure there is at least one attribute
        if (attributeCount > 0) {
            // Copy the first attribute (which is always OWNER_IP)
            EOS_LobbyDetails_CopyAttributeByIndexOptions attributeByIndexOptions = {};
            attributeByIndexOptions.ApiVersion = EOS_LOBBYDETAILS_COPYATTRIBUTEBYINDEX_API_LATEST;
            attributeByIndexOptions.AttrIndex = 0;  // Always retrieve the first attribute

            EOS_Lobby_Attribute* attribute = nullptr;
            EOS_EResult result = EOS_LobbyDetails_CopyAttributeByIndex(LobbyDetailsHandle, &attributeByIndexOptions, &attribute);

            if (result == EOS_EResult::EOS_Success && attribute) {
                // Ensure the attribute key is OWNER_IP
                if (std::string(attribute->Data->Key) == "OWNER_IP" && attribute->Data->ValueType == EOS_EAttributeType::EOS_AT_STRING) {
                    // Store the OWNER_IP in the class variable
                    ownerIP = attribute->Data->Value.AsUtf8;
                    std::cout << "OWNER_IP stored successfully!" << std::endl;
                }
                else {
                    std::cerr << "Error: First attribute is not OWNER_IP!" << std::endl;
                }

                // Free allocated memory for attribute
                EOS_Lobby_Attribute_Release(attribute);
            }
            else {
                std::cerr << "Error: Failed to retrieve OWNER_IP attribute." << std::endl;
            }
        }
        else {
            std::cerr << "Error: No attributes found in the lobby." << std::endl;
        }

        int ip1, ip2, ip3, ip4;
        if (sscanf_s(ownerIP.c_str(), "%d.%d.%d.%d", &ip1, &ip2, &ip3, &ip4) == 4) {
            std::cout << "IP Segments: " << ip1 << ", " << ip2 << ", " << ip3 << ", " << ip4 << std::endl;
        }
        else {
            std::cerr << "Error: Invalid IP format!" << std::endl;
        }

        // Retrieves the number of members in the lobby
        EOS_LobbyDetails_GetMemberCountOptions memberCountOptions = {};
        memberCountOptions.ApiVersion = EOS_LOBBYDETAILS_GETMEMBERCOUNT_API_LATEST;

        int32_t memberCount = EOS_LobbyDetails_GetMemberCount(LobbyDetailsHandle, &memberCountOptions);
        std::cout << "Other Members Count: " << (memberCount > 1 ? memberCount - 1 : 0) << std::endl;

        std::vector<EOS_ProductUserId> otherUsers;

        if (memberCount > 1)
        {
            for (int32_t i = 0; i < memberCount; ++i)
            {
                // Retrieves each member's ID
                EOS_LobbyDetails_GetMemberByIndexOptions memberByIndexOptions = {};
                memberByIndexOptions.ApiVersion = EOS_LOBBYDETAILS_GETMEMBERBYINDEX_API_LATEST;
                memberByIndexOptions.MemberIndex = i;

                EOS_ProductUserId memberId = EOS_LobbyDetails_GetMemberByIndex(LobbyDetailsHandle, &memberByIndexOptions);
                char memberStr[EOS_PRODUCTUSERID_MAX_LENGTH] = {};
                bufferSize = sizeof(memberStr);

                if (!EOS_ProductUserId_IsValid(memberId))
                {
                    std::cerr << "Error: Invalid ProductUserId retrieved for member " << i + 1 << std::endl;
                    continue;
                }

                EOS_EResult toStringResult = EOS_ProductUserId_ToString(memberId, memberStr, &bufferSize);
                if (toStringResult == EOS_EResult::EOS_Success)
                {
                    std::cout << "Member " << i << ": " << memberStr << std::endl;

                    // Store other user IDs for P2P communication
                    if (strcmp(localUserStr, memberStr) != 0) {
                        otherUsers.push_back(memberId);
                    }
                }
                else
                {
                    std::cerr << "Error: Failed to convert ProductUserId to string. Error code: " << static_cast<int>(toStringResult) << std::endl;
                }
            }
        }

        // Wait until all expected IPs have been received
        std::cout << "Waiting to collect all members' IP addresses..." << std::endl;

        std::cout << "All IP addresses received successfully!\n";
        for (const auto& entry : collectedIPs) {
            std::cout << "User " << entry.first << " -> IP: " << entry.second << "\n";
        }

        // Now all users have all IPs, and the game can proceed.
        if (isOwner)
        {
            std::cout << "Press 'P' to start the game..." << std::endl;
            char input;
            std::cin >> input;
            if (input == 'P' || input == 'p') {
                std::cout << "Game starting..." << std::endl;
            }

            /*
            * This should be when the server starts sending the packets to the users
            */

        }

        EOS_Platform_Tick(PlatformHandle);
        std::this_thread::sleep_for(std::chrono::seconds(5));
        eosSearchManager.CreateLobbySearch(lobbyInfo->LobbyId);
    }
}
