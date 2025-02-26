#include "lobby.h"
#include <iostream>
#include <thread>
#include <chrono>

EOS_HPlatform PlatformHandle = nullptr;
EOS_ProductUserId LocalUserId = nullptr;

void OnConnectLoginComplete(const EOS_Connect_LoginCallbackInfo* Data) {
    std::cout << "[OnConnectLoginComplete] Callback received." << std::endl;

    if (Data->ResultCode == EOS_EResult::EOS_Success) {
        LocalUserId = Data->LocalUserId;

        if (!LocalUserId) {
            std::cerr << "[ERROR] LocalUserId is NULL after successful login!" << std::endl;
            return;
        }

        if (!EOS_ProductUserId_IsValid(LocalUserId)) {
            std::cerr << "[ERROR] LocalUserId is invalid!" << std::endl;
            return;
        }

        char UserIdBuffer[EOS_PRODUCTUSERID_MAX_LENGTH + 1] = {};
        int32_t BufferSize = EOS_PRODUCTUSERID_MAX_LENGTH + 1;
        EOS_EResult res = EOS_ProductUserId_ToString(LocalUserId, UserIdBuffer, &BufferSize);

        if (res == EOS_EResult::EOS_Success) {
            std::cout << "[OnConnectLoginComplete] LocalUserId: " << UserIdBuffer << std::endl;
        }
        else {
            std::cerr << "[ERROR] Failed to convert LocalUserId to string. Error: "
                << EOS_EResult_ToString(res) << std::endl;
        }

        if (LocalUserId) {
            CreateLobby();
        }
        else {
            std::cerr << "[ERROR] LocalUserId is not valid. Cannot create a lobby." << std::endl;
        }
    }
    else {
        std::cerr << "[ERROR] EOS Connect Login Failed: " << EOS_EResult_ToString(Data->ResultCode) << std::endl;
    }
}



void OnAuthLoginComplete(const EOS_Auth_LoginCallbackInfo* Data) {
    std::cout << "[OnAuthLoginComplete] Callback received." << std::endl;

    if (Data->ResultCode == EOS_EResult::EOS_Success) {
        std::cout << "[OnAuthLoginComplete] Anonymous Login Successful!" << std::endl;

        EOS_HConnect ConnectHandle = EOS_Platform_GetConnectInterface(PlatformHandle);
        EOS_HAuth AuthHandle = EOS_Platform_GetAuthInterface(PlatformHandle);

        EOS_Auth_CopyUserAuthTokenOptions TokenOptions = {};
        TokenOptions.ApiVersion = EOS_AUTH_COPYUSERAUTHTOKEN_API_LATEST;

        EOS_Auth_Token* AuthToken = nullptr;
        EOS_EResult Result = EOS_Auth_CopyUserAuthToken(AuthHandle, &TokenOptions, Data->LocalUserId, &AuthToken);

        if (Result == EOS_EResult::EOS_Success && AuthToken) {
            std::cout << "[OnAuthLoginComplete] Retrieved Auth Token: " << AuthToken->AccessToken << std::endl;

            EOS_Connect_Credentials Credentials = {};
            Credentials.ApiVersion = EOS_CONNECT_CREDENTIALS_API_LATEST;
            Credentials.Token = AuthToken->AccessToken;
            Credentials.Type = EOS_EExternalCredentialType::EOS_ECT_EPIC;

            EOS_Connect_LoginOptions ConnectLoginOptions = {};
            ConnectLoginOptions.ApiVersion = EOS_CONNECT_LOGIN_API_LATEST;
            ConnectLoginOptions.Credentials = &Credentials;

            EOS_Connect_Login(ConnectHandle, &ConnectLoginOptions, nullptr, OnConnectLoginComplete);

            EOS_Auth_Token_Release(AuthToken);
        }
        else {
            std::cerr << "[ERROR] Failed to copy user auth token. Error: " << EOS_EResult_ToString(Result) << std::endl;
        }
    }
    else {
        std::cerr << "[ERROR] Anonymous Login Failed: " << EOS_EResult_ToString(Data->ResultCode) << std::endl;
    }
}

void LoginAnonymous() {
    std::cout << "[LoginAnonymous] Starting anonymous login..." << std::endl;

    EOS_HAuth AuthHandle = EOS_Platform_GetAuthInterface(PlatformHandle);

    EOS_Auth_Credentials Credentials = {};
    Credentials.ApiVersion = EOS_AUTH_CREDENTIALS_API_LATEST;
    Credentials.Type = EOS_ELoginCredentialType::EOS_LCT_PersistentAuth;

    EOS_Auth_LoginOptions LoginOptions = {};
    LoginOptions.ApiVersion = EOS_AUTH_LOGIN_API_LATEST;
    LoginOptions.Credentials = &Credentials;

    EOS_Auth_Login(AuthHandle, &LoginOptions, nullptr, OnAuthLoginComplete);
}


void OnLobbyCreated(const EOS_Lobby_CreateLobbyCallbackInfo* Data) {
    std::cout << "[OnLobbyCreated] Callback received." << std::endl;

    if (Data->ResultCode == EOS_EResult::EOS_Success) {
        std::cout << "[OnLobbyCreated] Lobby created successfully! Lobby ID: " << Data->LobbyId << std::endl;
    }
    else {
        std::cerr << "[ERROR] Failed to create lobby. Error: " << EOS_EResult_ToString(Data->ResultCode) << std::endl;

        if (Data->ResultCode == EOS_EResult::EOS_InvalidRequest) {
            std::cerr << "[DEBUG] Check if LocalUserId, PlatformHandle, and BucketId are valid." << std::endl;
        }
    }
}

void CreateLobby() {
    std::cout << "[CreateLobby] Attempting to create a lobby..." << std::endl;

    if (!LocalUserId) {
        std::cerr << "[ERROR] LocalUserId is NULL. Authentication must be complete before creating a lobby." << std::endl;
        return;
    }

    if (!PlatformHandle) {
        std::cerr << "[ERROR] PlatformHandle is NULL. Ensure the EOS platform is initialized." << std::endl;
        return;
    }

    if (!EOS_ProductUserId_IsValid(LocalUserId)) {
        std::cerr << "[ERROR] LocalUserId is invalid. Cannot create a lobby." << std::endl;
        return;
    }

    EOS_HLobby LobbyHandle = EOS_Platform_GetLobbyInterface(PlatformHandle);

    if (!LobbyHandle) {
        std::cerr << "[ERROR] LobbyHandle is NULL. Lobby interface might not be initialized." << std::endl;
        return;
    }

    EOS_Lobby_CreateLobbyOptions Options = {};
    Options.ApiVersion = EOS_LOBBY_CREATELOBBY_API_LATEST;
    Options.LocalUserId = LocalUserId;
    Options.MaxLobbyMembers = 4;
    Options.PermissionLevel = EOS_ELobbyPermissionLevel::EOS_LPL_PUBLICADVERTISED;
    Options.BucketId = "game_bucket";
    Options.bPresenceEnabled = EOS_TRUE;
    Options.bAllowInvites = EOS_TRUE;
    Options.bDisableHostMigration = EOS_FALSE;
    Options.bEnableRTCRoom = EOS_FALSE;
    Options.AllowedPlatformIdsCount = 0;
    Options.AllowedPlatformIds = nullptr;
    Options.bCrossplayOptOut = EOS_FALSE;

    std::cout << "[CreateLobby] LocalUserId: " << LocalUserId << std::endl;
    std::cout << "[CreateLobby] PlatformHandle: " << PlatformHandle << std::endl;
    std::cout << "[CreateLobby] LobbyHandle: " << LobbyHandle << std::endl;

    EOS_Lobby_CreateLobby(LobbyHandle, &Options, nullptr, OnLobbyCreated);
}

void StartEOS() {
    std::cout << "[StartEOS] Initializing EOS SDK..." << std::endl;

    EOS_InitializeOptions InitOptions = {};
    InitOptions.ApiVersion = EOS_INITIALIZE_API_LATEST;
    InitOptions.ProductName = "Uni Group Project";
    InitOptions.ProductVersion = "1.0";

    EOS_EResult InitResult = EOS_Initialize(&InitOptions);
    if (InitResult != EOS_EResult::EOS_Success) {
        std::cerr << "[ERROR] Failed to initialize EOS SDK: " << EOS_EResult_ToString(InitResult) << std::endl;
        return;
    }

    EOS_Platform_Options PlatformOptions = {};
    PlatformOptions.ApiVersion = EOS_PLATFORM_OPTIONS_API_LATEST;
    PlatformOptions.ProductId = "944c0fd27c634870ab2559503cfc2f03";
    PlatformOptions.SandboxId = "a1aba763c9a34a298e3386c3bcd724f4";
    PlatformOptions.DeploymentId = "44dcc0582fd84895a9f9663ad079c38c";
    PlatformOptions.ClientCredentials.ClientId = "xyza7891EdqWCp9ClnPHdgcPnqtVxvLJ";
    PlatformOptions.ClientCredentials.ClientSecret = "/FFpHHh7MxXXRmlxEk8oAr0vXy+tuQcvXNeLBp/7X4o";

    PlatformHandle = EOS_Platform_Create(&PlatformOptions);
    if (!PlatformHandle) {
        std::cerr << "[ERROR] Failed to create EOS Platform." << std::endl;
        return;
    }

    std::cout << "[StartEOS] EOS Platform initialized successfully." << std::endl;
    LoginAnonymous();

    while (true) {
        EOS_Platform_Tick(PlatformHandle);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
