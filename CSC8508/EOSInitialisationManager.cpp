#include "EOSInitialisationManager.h"
#include <eos_auth.h>
#include "EOSLobbyManager.h"
#include "EOSLobbySearch.h"

EOSInitialisationManager& EOSInitialisationManager::GetInstance() {
    static EOSInitialisationManager instance;
    return instance;
}

EOSInitialisationManager::EOSInitialisationManager() {
    std::cout << "[EOSInitialisationManager] Constructor called, but initialization deferred to StartEOS()." << std::endl;
}

// Initializes the EOS SDK and starts the login process
void EOSInitialisationManager::StartEOS() {
    std::cout << "[EOSInitialisationManager] Initializing EOS SDK..." << std::endl;

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

    std::cout << "[EOSInitialisationManager] EOS Platform initialized successfully." << std::endl;

    LoginAnonymous();
    while (true) {
        EOS_Platform_Tick(PlatformHandle);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

// Logs in the user anonymously
void EOSInitialisationManager::LoginAnonymous() {
    std::cout << "[LoginAnonymous] Starting anonymous login..." << std::endl;

    EOS_HAuth AuthHandle = EOS_Platform_GetAuthInterface(PlatformHandle);

    EOS_Auth_Credentials Credentials = {};
    Credentials.ApiVersion = EOS_AUTH_CREDENTIALS_API_LATEST;
    Credentials.Type = EOS_ELoginCredentialType::EOS_LCT_PersistentAuth;

    EOS_Auth_LoginOptions LoginOptions = {};
    LoginOptions.ApiVersion = EOS_AUTH_LOGIN_API_LATEST;
    LoginOptions.Credentials = &Credentials;

    EOS_Auth_Login(AuthHandle, &LoginOptions, this, OnAuthLoginComplete);
}

// Callback function for handling authentication login response
void EOSInitialisationManager::OnAuthLoginComplete(const EOS_Auth_LoginCallbackInfo* Data) {
    std::cout << "[OnAuthLoginComplete] Callback received." << std::endl;

    if (Data->ResultCode == EOS_EResult::EOS_Success) {
        std::cout << "[OnAuthLoginComplete] Anonymous Login Successful!" << std::endl;

        EOSInitialisationManager* instance = static_cast<EOSInitialisationManager*>(Data->ClientData);
        if (!instance) {
            std::cerr << "[ERROR] ClientData is null in OnAuthLoginComplete!" << std::endl;
            return;
        }

        EOS_HConnect ConnectHandle = EOS_Platform_GetConnectInterface(instance->PlatformHandle);
        EOS_HAuth AuthHandle = EOS_Platform_GetAuthInterface(instance->PlatformHandle);

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

            EOS_Connect_Login(ConnectHandle, &ConnectLoginOptions, instance, OnConnectLoginComplete);

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

// Callback function for handling connection login response
void EOSInitialisationManager::OnConnectLoginComplete(const EOS_Connect_LoginCallbackInfo* Data) {
    std::cout << "[OnConnectLoginComplete] Callback received." << std::endl;

    EOSInitialisationManager* instance = static_cast<EOSInitialisationManager*>(Data->ClientData);
    if (!instance) {
        std::cerr << "[ERROR] ClientData is null in OnConnectLoginComplete!" << std::endl;
        return;
    }

    if (Data->ResultCode == EOS_EResult::EOS_Success) {
        instance->LocalUserId = Data->LocalUserId;

        if (!instance->LocalUserId || !EOS_ProductUserId_IsValid(instance->LocalUserId)) {
            std::cerr << "[ERROR] LocalUserId is NULL or invalid after successful login!" << std::endl;
            return;
        }

        char UserIdBuffer[EOS_PRODUCTUSERID_MAX_LENGTH + 1] = {};
        int32_t BufferSize = EOS_PRODUCTUSERID_MAX_LENGTH + 1;
        EOS_EResult res = EOS_ProductUserId_ToString(instance->LocalUserId, UserIdBuffer, &BufferSize);

        if (res == EOS_EResult::EOS_Success) {
            std::cout << "[OnConnectLoginComplete] LocalUserId: " << UserIdBuffer << std::endl;
            EOSLobbySearch::GetInstance().CreateLobbySearch("2c011db3566a4228b3b3f82d8c363f66");
        }
        else {
            std::cerr << "[ERROR] Failed to convert LocalUserId to string. Error: "
                << EOS_EResult_ToString(res) << std::endl;
        }
    }
    else {
        std::cerr << "[ERROR] EOS Connect Login Failed: " << EOS_EResult_ToString(Data->ResultCode) << std::endl;
    }
}

// Returns the EOS platform handle
EOS_HPlatform EOSInitialisationManager::GetPlatformHandle() const {
    return PlatformHandle;
}

// Returns the local user ID
EOS_ProductUserId EOSInitialisationManager::GetLocalUserId() const {
    return LocalUserId;
}

// Destructor - Releases the EOS platform handle
EOSInitialisationManager::~EOSInitialisationManager() {
    if (PlatformHandle) {
        EOS_Platform_Release(PlatformHandle);
        PlatformHandle = nullptr;
    }
}
