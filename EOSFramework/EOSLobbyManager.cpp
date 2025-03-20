#include "EOSLobbyManager.h"
#include "EOSLobbySearch.h"
#include "EOSLobbyFunctions.h"

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)


//Creates a new lobby and sets its attributes
void EOSLobbyManager::CreateLobby() {
    std::cout << "[CreateLobby] Attempting to create a lobby..." << std::endl;

    // Ensures the user is authenticated before creating a lobby
    if (!eosManager.LocalUserId) {
        std::cerr << "[ERROR] LocalUserId is NULL. Authentication must be complete before creating a lobby." << std::endl;
        return;
    }

    EOS_HPlatform platformHandle = eosManager.GetPlatformHandle();

    // Ensures the EOS platform is initialized before proceeding
    if (!platformHandle) {
        std::cerr << "[ERROR] PlatformHandle is NULL. Ensure the EOS platform is initialized." << std::endl;
        return;
    }

    // Validates the LocalUserId before creating a lobby
    if (!EOS_ProductUserId_IsValid(eosManager.LocalUserId)) {
        std::cerr << "[ERROR] LocalUserId is invalid. Cannot create a lobby." << std::endl;
        return;
    }

    // Retrieves the EOS Lobby interface handle
    LobbyHandle = EOS_Platform_GetLobbyInterface(platformHandle);

    // Ensures the lobby interface is initialised properly
    if (!LobbyHandle) {
        std::cerr << "[ERROR] LobbyHandle is NULL. Lobby interface might not be initialized." << std::endl;
        return;
    }

    // Configures the options for creating a new lobby
    EOS_Lobby_CreateLobbyOptions CreateOptions = {};
    CreateOptions.ApiVersion = EOS_LOBBY_CREATELOBBY_API_LATEST;
    CreateOptions.LocalUserId = eosManager.LocalUserId;
    CreateOptions.MaxLobbyMembers = 4;
    CreateOptions.PermissionLevel = EOS_ELobbyPermissionLevel::EOS_LPL_PUBLICADVERTISED;
    CreateOptions.BucketId = "game_bucket";
    CreateOptions.bPresenceEnabled = EOS_TRUE;
    CreateOptions.bAllowInvites = EOS_TRUE;
    CreateOptions.bDisableHostMigration = EOS_FALSE;
    CreateOptions.bEnableRTCRoom = EOS_FALSE;
    CreateOptions.AllowedPlatformIdsCount = 0;
    CreateOptions.AllowedPlatformIds = nullptr;
    CreateOptions.bCrossplayOptOut = EOS_FALSE;

    // Initiates the request to create the lobby
    EOS_Lobby_CreateLobby(LobbyHandle, &CreateOptions, nullptr, OnLobbyCreated);

    std::cout << "[CreateLobby] Lobby created with attribute 'Lobby: Active' set to true." << std::endl;
}

//Callback function triggered when a lobby is created
void EOSLobbyManager::OnLobbyCreated(const EOS_Lobby_CreateLobbyCallbackInfo* Data) {
    std::cout << "[OnLobbyCreated] Callback received." << std::endl;
    EOSLobbyManager* self = static_cast<EOSLobbyManager*>(Data->ClientData); // Cast back to instance

    self->LobbyHandle = EOS_Platform_GetLobbyInterface(self->eosManager.GetPlatformHandle());

    // Checks if the lobby creation was successful
    if (Data->ResultCode == EOS_EResult::EOS_Success) {
        std::cout << "[OnLobbyCreated] Lobby created successfully! Lobby ID: " << Data->LobbyId << ". You are now the owner!" << std::endl;

        // Initialises the Lobby Modification Handle
        EOS_HLobbyModification LobbyModificationHandle = nullptr;
        EOS_Lobby_UpdateLobbyModificationOptions ModOptions = {};
        ModOptions.ApiVersion = EOS_LOBBY_UPDATELOBBYMODIFICATION_API_LATEST;
        ModOptions.LobbyId = Data->LobbyId;
        ModOptions.LocalUserId = self->eosManager.GetLocalUserId();

        // Requests a modification handle to update lobby attributes
        EOS_EResult Result = EOS_Lobby_UpdateLobbyModification(self->LobbyHandle, &ModOptions, &LobbyModificationHandle);
        if (Result == EOS_EResult::EOS_Success) {
           // const std::string PUBLIC_IP_ADDRESS = "128.366.543.43"; // fake ip
            std::cout << "[OnLobbyCreated] Lobby Modification Handle successfully created." << std::endl;
            std::cout << "Public IP Address: " << TOSTRING(PUBLIC_IP_ADDRESS) << std::endl;

            // Stores the lobby ID in the manager
            strncpy_s(self->LobbyId, Data->LobbyId, 256 - 1);
            self->LobbyId[256 - 1] = '\0';  // Ensure null

            // Defines a new attribute for the lobby
            EOS_Lobby_AttributeData lobbyAttributeData = {};
            lobbyAttributeData.ApiVersion = EOS_LOBBY_ATTRIBUTEDATA_API_LATEST;
            lobbyAttributeData.ValueType = EOS_ELobbyAttributeType::EOS_AT_STRING;

            // Sets the attribute key and value to the owner's IP address
            lobbyAttributeData.Key = "OWNER_IP";
            lobbyAttributeData.Value.AsUtf8 = TOSTRING(PUBLIC_IP_ADDRESS); // Assuming PUBLIC_IP_ADDRESS is defined correctly

            // Configures the lobby attribute for visibility
            EOS_Lobby_Attribute lobbyAttribute = {};
            lobbyAttribute.ApiVersion = EOS_LOBBY_ATTRIBUTE_API_LATEST;
            lobbyAttribute.Data = &lobbyAttributeData;
            lobbyAttribute.Visibility = EOS_ELobbyAttributeVisibility::EOS_LAT_PUBLIC;

            // Configures the lobby modification attribute
            EOS_LobbyModification_AddAttributeOptions lobbyModAttribute = {};
            lobbyModAttribute.ApiVersion = EOS_LOBBYMODIFICATION_ADDATTRIBUTE_API_LATEST;
            lobbyModAttribute.Visibility = EOS_ELobbyAttributeVisibility::EOS_LAT_PUBLIC;
            lobbyModAttribute.Attribute = &lobbyAttributeData;

            // Adds the attribute to the lobby modification handle
            EOS_EResult Result = EOS_LobbyModification_AddAttribute(LobbyModificationHandle, &lobbyModAttribute);
            if (Result == EOS_EResult::EOS_Success) {
                std::cout << "[OnLobbyCreated] Attribute 'OWNER_IP' set to: " << TOSTRING(PUBLIC_IP_ADDRESS) << " successfully." << std::endl;
            }
            else {
                std::cerr << "[ERROR] Failed to add 'OWNER_IP' attribute. Error: " << EOS_EResult_ToString(Result) << std::endl;
            }

            // Configures the update options for modifying the lobby
            EOS_Lobby_UpdateLobbyOptions UpdateOptions = {};
            UpdateOptions.ApiVersion = EOS_LOBBY_UPDATELOBBY_API_LATEST;
            UpdateOptions.LobbyModificationHandle = LobbyModificationHandle;

            // Applies the lobby update
            EOS_Lobby_UpdateLobby(self->LobbyHandle, &UpdateOptions, nullptr, OnLobbyUpdated);

            // Releases the lobby modification handle as it is no longer needed
            EOS_LobbyModification_Release(LobbyModificationHandle);
        }
        else {
            std::cerr << "[ERROR] Failed to create Lobby Modification Handle. Error: " << EOS_EResult_ToString(Result) << std::endl;
        }
    }
    else {
        std::cerr << "[ERROR] Failed to create lobby. Error: " << EOS_EResult_ToString(Data->ResultCode) << std::endl;

        if (Data->ResultCode == EOS_EResult::EOS_InvalidRequest) {
            std::cerr << "[DEBUG] Check if LocalUserId, PlatformHandle, and BucketId are valid." << std::endl;
        }
    }
}

// Callback function triggered when a lobby update is completed
void EOSLobbyManager::OnLobbyUpdated(const EOS_Lobby_UpdateLobbyCallbackInfo* Data) {
    std::cout << "[OnLobbyUpdated] Callback received." << std::endl;

    EOSLobbyManager* self = static_cast<EOSLobbyManager*>(Data->ClientData); // Cast back to instance

    if (Data->ResultCode == EOS_EResult::EOS_Success) {
        std::cout << "[OnLobbyUpdated] Lobby update successful!" << std::endl;

        self->lobbyCreated = true;
    }
    else {
        std::cerr << "[ERROR] Lobby update failed. Error: " << EOS_EResult_ToString(Data->ResultCode) << std::endl;
    }
}

// Retrieves the handle for the current lobby
EOS_HLobby EOSLobbyManager::GetLobbyHandle() const {
    return LobbyHandle;
}
