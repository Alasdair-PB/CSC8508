#include "EOSLobbyFunctions.h"
#include "EOSInitialisationManager.h"
#include "EOSLobbyManager.h"
#include "EOSLobbySearch.h"
#include <unordered_map>

EOSLobbyFunctions::EOSLobbyFunctions(EOSInitialisationManager& initManager, EOSLobbySearch& lobbySearch)
    : eosInitManager(initManager), eosSearchManager(lobbySearch) {
}

// Destructor
EOSLobbyFunctions::~EOSLobbyFunctions() {
}

// Attempts to join a lobby
void EOSLobbyFunctions::JoinLobby() {
    // Ensures that the LocalUserId is valid before attempting to join the lobby
    if (eosInitManager.LocalUserId == nullptr) {
        std::cerr << "[ERROR] LocalUserId is null, cannot join lobby\n";
        return;
    }

    // Ensures that a valid lobby details handle is available
    if (eosSearchManager.LobbyDetailsHandle == nullptr) {
        std::cerr << "[ERROR] Invalid LobbyDetailsHandle before JoinLobby()\n";
        return;
    }

    // Ensures that a valid lobby handle is available
    if (eosSearchManager.LobbyHandle == nullptr) {
        std::cerr << "[ERROR] Invalid LobbyHandle before JoinLobby()\n";
        return;
    }

    // Configures the options for joining a lobby
    EOS_Lobby_JoinLobbyOptions JoinOptions = {};
    JoinOptions.ApiVersion = EOS_LOBBY_JOINLOBBY_API_LATEST;
    JoinOptions.LobbyDetailsHandle = eosSearchManager.LobbyDetailsHandle;
    JoinOptions.LocalUserId = eosInitManager.LocalUserId;
    JoinOptions.LocalRTCOptions = NULL;
    JoinOptions.bPresenceEnabled = EOS_TRUE;
    JoinOptions.bCrossplayOptOut = EOS_FALSE;
    JoinOptions.RTCRoomJoinActionType = EOS_ELobbyRTCRoomJoinActionType::EOS_LRRJAT_AutomaticJoin;

    // Initiates the lobby join request
    EOS_Lobby_JoinLobby(eosSearchManager.LobbyHandle, &JoinOptions, this, EOSLobbyFunctions::OnJoinLobbyComplete);

    running = true;

    RunUpdateLoop();

}


// New function to keep updating EOS
void EOSLobbyFunctions::RunUpdateLoop()
{
    while (running) {
        if (eosInitManager.PlatformHandle) {
            EOS_Platform_Tick(eosInitManager.PlatformHandle);
        }
    }
}

// Callback function triggered when the attempt to join a lobby completes
void EOSLobbyFunctions::OnJoinLobbyComplete(const EOS_Lobby_JoinLobbyCallbackInfo* Data) {
if (Data->ResultCode == EOS_EResult::EOS_Success) {
        std::cout << "[OnJoinLobbyComplete] Successfully joined lobby.\n";

        EOSLobbyFunctions* self = static_cast<EOSLobbyFunctions*>(Data->ClientData); // Cast back to instance
        self->running = false;
    }
    else {
        std::cerr << "[ERROR] Failed to join lobby. Error: " << EOS_EResult_ToString(Data->ResultCode) << std::endl;
    }

   

}

// Attempts to leave a lobby
void EOSLobbyFunctions::LeaveLobby() {
    // Configures the options for leaving a lobby
    EOS_Lobby_LeaveLobbyOptions LeaveOptions = {};
    LeaveOptions.ApiVersion = EOS_LOBBY_LEAVELOBBY_API_LATEST;
    LeaveOptions.LocalUserId = eosInitManager.LocalUserId;
    LeaveOptions.LobbyId = "8b04b581a080487c98ee5ee27338e65b";

    // Initiates the request to leave the lobby
    EOS_Lobby_LeaveLobby(eosSearchManager.LobbyHandle, &LeaveOptions, nullptr, OnLeaveLobbyComplete);
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
    EOS_HLobbyDetails LobbyDetailsHandle = eosSearchManager.GetLobbyDetailsHandle();

    if (!ValidateLobbyDetailsHandle(LobbyDetailsHandle)) return;

    EOS_LobbyDetails_Info* lobbyInfo = CopyLobbyInfo(LobbyDetailsHandle);
    if (!lobbyInfo) return;

    eosSearchManager.CreateLobbySearch(lobbyInfo->LobbyId);
    lobbyID = lobbyInfo->LobbyId;

    bool isOwner = DetermineIfOwner(lobbyInfo->LobbyOwnerUserId);

    ownerIP = GetOwnerIPAttribute(LobbyDetailsHandle);
    if (!ownerIP.empty()) {
        ParseOwnerIP(ownerIP);
    }

    std::vector<EOS_ProductUserId> otherUsers = GetLobbyMembers(LobbyDetailsHandle);
}

bool EOSLobbyFunctions::ValidateLobbyDetailsHandle(EOS_HLobbyDetails handle)
{
    if (!handle)
    {
        std::cerr << "Error: Invalid lobby details handle." << std::endl;
        return false;
    }
    return true;
}

EOS_LobbyDetails_Info* EOSLobbyFunctions::CopyLobbyInfo(EOS_HLobbyDetails handle)
{
    EOS_LobbyDetails_CopyInfoOptions options = { EOS_LOBBYDETAILS_COPYINFO_API_LATEST };
    EOS_LobbyDetails_Info* info = nullptr;
    EOS_EResult result = EOS_LobbyDetails_CopyInfo(handle, &options, &info);

    if (result != EOS_EResult::EOS_Success || !info)
    {
        std::cerr << "Error: Failed to copy lobby info." << std::endl;
        return nullptr;
    }

    std::cout << "Lobby ID: " << info->LobbyId << std::endl;
    std::cout << "Max Members: " << info->MaxMembers << std::endl;
    return info;
}

bool EOSLobbyFunctions::DetermineIfOwner(EOS_ProductUserId ownerId)
{
    EOS_ProductUserId localUserId = eosInitManager.GetLocalUserId();
    char ownerStr[EOS_PRODUCTUSERID_MAX_LENGTH + 1] = {};
    char localStr[EOS_PRODUCTUSERID_MAX_LENGTH + 1] = {};
    int32_t bufferSize = sizeof(ownerStr);

    if (EOS_ProductUserId_ToString(ownerId, ownerStr, &bufferSize) != EOS_EResult::EOS_Success)
    {
        std::cerr << "Failed to convert LobbyOwnerUserId to string!" << std::endl;
        return false;
    }

    bufferSize = sizeof(localStr);
    if (!EOS_ProductUserId_IsValid(localUserId) ||
        EOS_ProductUserId_ToString(localUserId, localStr, &bufferSize) != EOS_EResult::EOS_Success)
    {
        std::cerr << "Error converting local user ID to string." << std::endl;
        return false;
    }

    std::cout << "Lobby owner: " << ownerStr << "\nYou (Local User): " << localStr << std::endl;
    return strcmp(localStr, ownerStr) == 0;
}

std::string EOSLobbyFunctions::GetOwnerIPAttribute(EOS_HLobbyDetails handle)
{
    EOS_LobbyDetails_GetAttributeCountOptions countOptions = { EOS_LOBBYDETAILS_GETATTRIBUTECOUNT_API_LATEST };
    int32_t count = EOS_LobbyDetails_GetAttributeCount(handle, &countOptions);

    if (count <= 0) {
        std::cerr << "Error: No attributes found in the lobby." << std::endl;
        return "";
    }

    EOS_LobbyDetails_CopyAttributeByIndexOptions attrOptions = { EOS_LOBBYDETAILS_COPYATTRIBUTEBYINDEX_API_LATEST, 0 };
    EOS_Lobby_Attribute* attribute = nullptr;
    EOS_EResult result = EOS_LobbyDetails_CopyAttributeByIndex(handle, &attrOptions, &attribute);

    std::string ip;
    if (result == EOS_EResult::EOS_Success && attribute)
    {
        if (std::string(attribute->Data->Key) == "OWNER_IP" && attribute->Data->ValueType == EOS_EAttributeType::EOS_AT_STRING)
        {
            ip = attribute->Data->Value.AsUtf8;
            std::cout << "OWNER_IP stored successfully!" << std::endl;
        }
        else {
            std::cerr << "Error: First attribute is not OWNER_IP!" << std::endl;
        }

        EOS_Lobby_Attribute_Release(attribute);
    }
    else {
        std::cerr << "Error: Failed to retrieve OWNER_IP attribute." << std::endl;
    }

    return ip;
}

void EOSLobbyFunctions::ParseOwnerIP(const std::string& ip)
{
    int ip1, ip2, ip3, ip4;
    if (sscanf_s(ip.c_str(), "%d.%d.%d.%d", &ip1, &ip2, &ip3, &ip4) == 4)
    {
        std::cout << "IP Segments: " << ip1 << ", " << ip2 << ", " << ip3 << ", " << ip4 << std::endl;
    }
    else
    {
        std::cerr << "Error: Invalid IP format!" << std::endl;
    }
}

std::vector<EOS_ProductUserId> EOSLobbyFunctions::GetLobbyMembers(EOS_HLobbyDetails handle)
{
    std::vector<EOS_ProductUserId> otherUsers;

    EOS_LobbyDetails_GetMemberCountOptions countOptions = { EOS_LOBBYDETAILS_GETMEMBERCOUNT_API_LATEST };
    int32_t playerCount = EOS_LobbyDetails_GetMemberCount(handle, &countOptions);

    EOS_ProductUserId localUserId = eosInitManager.GetLocalUserId();
    char localUserStr[EOS_PRODUCTUSERID_MAX_LENGTH] = {};
    int32_t bufferSize = sizeof(localUserStr);
    EOS_ProductUserId_ToString(localUserId, localUserStr, &bufferSize);

    for (int32_t i = 0; i < playerCount; ++i)
    {
        EOS_LobbyDetails_GetMemberByIndexOptions indexOptions = { EOS_LOBBYDETAILS_GETMEMBERBYINDEX_API_LATEST, i };
        EOS_ProductUserId memberId = EOS_LobbyDetails_GetMemberByIndex(handle, &indexOptions);

        if (!EOS_ProductUserId_IsValid(memberId))
        {
            std::cerr << "Error: Invalid ProductUserId for member " << i << std::endl;
            continue;
        }

        char memberStr[EOS_PRODUCTUSERID_MAX_LENGTH] = {};
        bufferSize = sizeof(memberStr);
        if (EOS_ProductUserId_ToString(memberId, memberStr, &bufferSize) == EOS_EResult::EOS_Success)
        {
            std::cout << "Member " << i << ": " << memberStr << std::endl;
            if (strcmp(localUserStr, memberStr) != 0) {
                otherUsers.push_back(memberId);
            }
        }
    }

    return otherUsers;
}