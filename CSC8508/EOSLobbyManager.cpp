#include "EOSLobbyManager.h"
#include "EOSLobbySearch.h"
#include "EOSIPDistribution.h"
#include "EOSLobbyFunctions.h"

EOSLobbyManager& EOSLobbyManager::GetInstance() {
    static EOSLobbyManager instance;
    return instance;
}

EOSLobbyManager::EOSLobbyManager() {
    std::cout << "[EOSLobbyManager] Constructor called." << std::endl;
}

EOSLobbyManager::~EOSLobbyManager() {
    if (LobbyHandle) {
        LobbyHandle = nullptr;
    }
}

//Creates a new lobby and sets its attributes
void EOSLobbyManager::CreateLobby() {
    std::cout << "[CreateLobby] Attempting to create a lobby..." << std::endl;

    EOSInitialisationManager& eosManager = EOSInitialisationManager::GetInstance();
    EOS_ProductUserId LocalUserId = eosManager.GetLocalUserId();
    EOS_HPlatform PlatformHandle = eosManager.GetPlatformHandle();

    if (!LocalUserId) {
        std::cerr << "[ERROR] LocalUserId is NULL. Authentication must be complete before creating a lobby." << std::endl;
        return;
    }

    EOS_HPlatform platformHandle = eosManager.GetPlatformHandle();

    if (!platformHandle) {
        std::cerr << "[ERROR] PlatformHandle is NULL. Ensure the EOS platform is initialized." << std::endl;
        return;
    }

    if (!EOS_ProductUserId_IsValid(LocalUserId)) {
        std::cerr << "[ERROR] LocalUserId is invalid. Cannot create a lobby." << std::endl;
        return;
    }

    LobbyHandle = EOS_Platform_GetLobbyInterface(platformHandle);

    if (!LobbyHandle) {
        std::cerr << "[ERROR] LobbyHandle is NULL. Lobby interface might not be initialized." << std::endl;
        return;
    }

    EOS_HLobbyModification LobbyModificationHandle = nullptr;

    EOS_Lobby_CreateLobbyOptions CreateOptions = {};
    CreateOptions.ApiVersion = EOS_LOBBY_CREATELOBBY_API_LATEST;
    CreateOptions.LocalUserId = LocalUserId;
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

    EOS_Lobby_CreateLobby(LobbyHandle, &CreateOptions, nullptr, OnLobbyCreated);

    std::cout << "[CreateLobby] Lobby created with attribute 'Lobby: Active' set to true." << std::endl;

    while (!lobbyCreated) { // Add a flag to control the loop
        EOS_Platform_Tick(PlatformHandle);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

//Callback function triggered when a lobby is created
void EOSLobbyManager::OnLobbyCreated(const EOS_Lobby_CreateLobbyCallbackInfo* Data) {
    std::cout << "[OnLobbyCreated] Callback received." << std::endl;

    EOSInitialisationManager& eosManager = EOSInitialisationManager::GetInstance();
    

    EOSLobbyManager& eosLobbyManager = EOSLobbyManager::GetInstance();
    eosLobbyManager.LobbyHandle = EOS_Platform_GetLobbyInterface(eosManager.GetPlatformHandle());

    if (Data->ResultCode == EOS_EResult::EOS_Success) {
        std::cout << "[OnLobbyCreated] Lobby created successfully! Lobby ID: " << Data->LobbyId << ". You are now the owner!" << std::endl;

        EOS_HLobbyModification LobbyModificationHandle = nullptr;
        EOS_Lobby_UpdateLobbyModificationOptions ModOptions = {};
        ModOptions.ApiVersion = EOS_LOBBY_UPDATELOBBYMODIFICATION_API_LATEST;
        ModOptions.LobbyId = Data->LobbyId;
        ModOptions.LocalUserId = eosManager.GetLocalUserId();

        EOS_EResult Result = EOS_Lobby_UpdateLobbyModification(eosLobbyManager.LobbyHandle, &ModOptions, &LobbyModificationHandle);
        if (Result == EOS_EResult::EOS_Success) {
            std::cout << "[OnLobbyCreated] Lobby Modification Handle successfully created." << std::endl;

            EOSLobbyManager& eosLobbyManager = EOSLobbyManager::GetInstance();
            strncpy_s(eosLobbyManager.LobbyId, Data->LobbyId, 256 - 1);
            eosLobbyManager.LobbyId[256 - 1] = '\0';  // Ensure null

            EOS_Lobby_AttributeData lobbyAttributeData = {};
            lobbyAttributeData.ApiVersion = EOS_LOBBY_ATTRIBUTEDATA_API_LATEST;
            lobbyAttributeData.ValueType = EOS_ELobbyAttributeType::EOS_AT_STRING;

            lobbyAttributeData.Key = "LOBBYSERVICEATTRIBUTE1";
            lobbyAttributeData.Value.AsUtf8 = "SEARCHKEYWORDS";

            EOS_Lobby_Attribute lobbyAttribute = {};
            lobbyAttribute.ApiVersion = EOS_LOBBY_ATTRIBUTE_API_LATEST;
            lobbyAttribute.Data = &lobbyAttributeData;
            lobbyAttribute.Visibility = EOS_ELobbyAttributeVisibility::EOS_LAT_PUBLIC;

            EOS_LobbyModification_AddAttributeOptions lobbyModAttribute = {};
            lobbyModAttribute.ApiVersion = EOS_LOBBYMODIFICATION_ADDATTRIBUTE_API_LATEST;
            lobbyModAttribute.Visibility = EOS_ELobbyAttributeVisibility::EOS_LAT_PUBLIC;
            lobbyModAttribute.Attribute = &lobbyAttributeData;

            EOS_EResult Result = EOS_LobbyModification_AddAttribute(LobbyModificationHandle, &lobbyModAttribute);
            if (Result == EOS_EResult::EOS_Success) {
                std::cout << "[OnLobbyCreated] Attribute 'LobbyStatus: Active' added successfully." << std::endl;
            }
            else {
                std::cerr << "[ERROR] Failed to add attribute. Error: " << EOS_EResult_ToString(Result) << std::endl;
            }

            EOS_Lobby_UpdateLobbyOptions UpdateOptions = {};
            UpdateOptions.ApiVersion = EOS_LOBBY_UPDATELOBBY_API_LATEST;
            UpdateOptions.LobbyModificationHandle = LobbyModificationHandle;

            EOS_Lobby_UpdateLobby(eosLobbyManager.LobbyHandle, &UpdateOptions, nullptr, OnLobbyUpdated);

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

void EOSLobbyManager::OnLobbyUpdated(const EOS_Lobby_UpdateLobbyCallbackInfo* Data) {
    std::cout << "[OnLobbyUpdated] Callback received." << std::endl;

    if (Data->ResultCode == EOS_EResult::EOS_Success) {
        std::cout << "[OnLobbyUpdated] Lobby update successful!" << std::endl;
        EOSLobbyManager& eosLobbyManager = EOSLobbyManager::GetInstance();
        eosLobbyManager.lobbyCreated = true;  // <-- Unblocks the loop in CreateLobby()
    }
    else {
        std::cerr << "[ERROR] Lobby update failed. Error: " << EOS_EResult_ToString(Data->ResultCode) << std::endl;
    }
}

EOS_HLobby EOSLobbyManager::GetLobbyHandle() const {
    return LobbyHandle;
}
