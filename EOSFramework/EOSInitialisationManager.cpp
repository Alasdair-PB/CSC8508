#if EOSBUILD

#include "EOSInitialisationManager.h"
#include <eos_auth.h>
#include "EOSLobbyManager.h"
#include "EOSLobbySearch.h"

// Constructor - Defers initialisation to StartEOS()
EOSInitialisationManager::EOSInitialisationManager() {
    std::cout << "[EOSInitialisationManager] Constructor called, but initialization deferred to StartEOS()." << std::endl;
}

// Destructor - Releases the EOS platform handle
EOSInitialisationManager::~EOSInitialisationManager() {
}

// Initializes the EOS SDK and starts the login process
void EOSInitialisationManager::StartEOS() {
    std::cout << "[EOSInitialisationManager] Initializing EOS SDK..." << std::endl;
    
    // Assigns the onAuthComplete function to authCompleteCallback for handling authentication completion

    // Creates a initialisation handler and sets its options
    EOS_InitializeOptions InitOptions = {};
    InitOptions.ApiVersion = EOS_INITIALIZE_API_LATEST;
    InitOptions.ProductName = "Uni Group Project";
    InitOptions.ProductVersion = "1.0";

    // Initialises EOS using the options and provides a message upon completion
    EOS_EResult InitResult = EOS_Initialize(&InitOptions);
    if (InitResult != EOS_EResult::EOS_Success) {
        std::cerr << "[ERROR] Failed to initialize EOS SDK: " << EOS_EResult_ToString(InitResult) << std::endl;
        return;
    }

    // Creates a handler for the EOS platform options, which link this project to the browser
    EOS_Platform_Options PlatformOptions = {};
    PlatformOptions.ApiVersion = EOS_PLATFORM_OPTIONS_API_LATEST;
    PlatformOptions.ProductId = "944c0fd27c634870ab2559503cfc2f03";
    PlatformOptions.SandboxId = "a1aba763c9a34a298e3386c3bcd724f4";
    PlatformOptions.DeploymentId = "44dcc0582fd84895a9f9663ad079c38c";
    PlatformOptions.ClientCredentials.ClientId = "xyza7891EdqWCp9ClnPHdgcPnqtVxvLJ";
    PlatformOptions.ClientCredentials.ClientSecret = "/FFpHHh7MxXXRmlxEk8oAr0vXy+tuQcvXNeLBp/7X4o";

    // Creates a platform handle, along with a message upom its completion
    PlatformHandle = EOS_Platform_Create(&PlatformOptions);

    running = true;

    std::cout << "Platform Handle set";

    if (!PlatformHandle) {
        std::cerr << "[ERROR] Failed to create EOS Platform." << std::endl;
        return;
    }

    std::cout << "[EOSInitialisationManager] EOS Platform initialized successfully." << std::endl;

    LoginAnonymous();

    RunUpdateLoop();
}


// Logs the user in anonymously with their authorised Epic Online Services account
void EOSInitialisationManager::LoginAnonymous() {
    std::cout << "[LoginAnonymous] Attempting login with PersistentAuth..." << std::endl;


    EOS_HAuth AuthHandle = EOS_Platform_GetAuthInterface(PlatformHandle);

    // Fetches the credentials that have previously been used
    EOS_Auth_Credentials Credentials = {};
    Credentials.ApiVersion = EOS_AUTH_CREDENTIALS_API_LATEST;

    // Change this so when not first time login, use *PersistentLogin*
    Credentials.Type = EOS_ELoginCredentialType::EOS_LCT_AccountPortal;

    EOS_Auth_LoginOptions LoginOptions = {};
    LoginOptions.ApiVersion = EOS_AUTH_LOGIN_API_LATEST;
    LoginOptions.Credentials = &Credentials;

    // Logs the user in
    EOS_Auth_Login(AuthHandle, &LoginOptions, this, OnAuthLoginComplete);
}

// New function to keep updating EOS
void EOSInitialisationManager::RunUpdateLoop() 
{
    while (running) {
        if (PlatformHandle) {
            EOS_Platform_Tick(PlatformHandle);
        }
    }
}


// Callback method from the user login
void EOSInitialisationManager::OnAuthLoginComplete(const EOS_Auth_LoginCallbackInfo* Data) {
    std::cout << "[OnAuthLoginComplete] Callback received." << std::endl;

    // Retrieve the EOSInitialisationManager instance from ClientData
    EOSInitialisationManager* instance = static_cast<EOSInitialisationManager*>(Data->ClientData);

    // Checks if instance is null
    if (!instance) {
        std::cerr << "[ERROR] ClientData is null in OnAuthLoginComplete!" << std::endl;
        return;
    }

    // If the login is successful, handle the next process of the login
    if (Data->ResultCode == EOS_EResult::EOS_Success) {
        std::cout << "[OnAuthLoginComplete] Login Successful using PersistentAuth!" << std::endl;
        instance->HandleSuccessfulLogin(Data);
    }
    // If not successful, try another method of logging in
    else if (Data->ResultCode == EOS_EResult::EOS_InvalidUser || Data->ResultCode == EOS_EResult::EOS_NotFound) {
        std::cout << "[OnAuthLoginComplete] PersistentAuth failed. Trying AccountPortal login..." << std::endl;
        instance->LoginWithAccountPortal();
    }
    else {
        std::cerr << "[ERROR] Login Failed: " << EOS_EResult_ToString(Data->ResultCode) << std::endl;
    }


}


// Fallback function to use AccountPortal if PersistentAuth fails
void EOSInitialisationManager::LoginWithAccountPortal() {
    std::cout << "[LoginWithAccountPortal] Attempting login via Account Portal..." << std::endl;

    EOS_HAuth AuthHandle = EOS_Platform_GetAuthInterface(PlatformHandle);
    
    // Login with account portal if this is the first time logging in
    EOS_Auth_Credentials Credentials = {};
    Credentials.ApiVersion = EOS_AUTH_CREDENTIALS_API_LATEST;
    Credentials.Type = EOS_ELoginCredentialType::EOS_LCT_AccountPortal; // Use Account Portal

    EOS_Auth_LoginOptions LoginOptions = {};
    LoginOptions.ApiVersion = EOS_AUTH_LOGIN_API_LATEST;
    LoginOptions.Credentials = &Credentials;

    // Logs the user in
    EOS_Auth_Login(AuthHandle, &LoginOptions, this, OnAuthLoginComplete);
}

// Handles successful login logic
void EOSInitialisationManager::HandleSuccessfulLogin(const EOS_Auth_LoginCallbackInfo* Data) {
    // Retrieves the EOSInitialisationManager instance
    EOSInitialisationManager* instance = static_cast<EOSInitialisationManager*>(Data->ClientData);

    if (!instance) {
        std::cerr << "[ERROR] ClientData is null in HandleSuccessfulLogin!" << std::endl;
        return;
    }

    if (!instance->PlatformHandle) {
        std::cerr << "[ERROR] PlatformHandle is NULL in HandleSuccessfulLogin!" << std::endl;
        return;
    }

    if (!Data->LocalUserId) {
        std::cerr << "[ERROR] LocalUserId is NULL in HandleSuccessfulLogin!" << std::endl;
        return;
    }


    // Retrieves the EOS Connect and Auth interfaces from the EOS platform handle
    EOS_HConnect ConnectHandle = EOS_Platform_GetConnectInterface(instance->PlatformHandle);
    EOS_HAuth AuthHandle = EOS_Platform_GetAuthInterface(instance->PlatformHandle);

    // Sets up options for copying the user's authentication token
    EOS_Auth_CopyUserAuthTokenOptions TokenOptions = {};
    TokenOptions.ApiVersion = EOS_AUTH_COPYUSERAUTHTOKEN_API_LATEST;

    EOS_Auth_Token* AuthToken = nullptr;

    // Copies the user authentication token from EOS Auth interface.
    EOS_EResult Result = EOS_Auth_CopyUserAuthToken(AuthHandle, &TokenOptions, Data->LocalUserId, &AuthToken);

    // Checks if the authentication token was retrieved successfully
    if (Result == EOS_EResult::EOS_Success && AuthToken) {
        std::cout << "[HandleSuccessfulLogin] Retrieved Auth Token: " << AuthToken->AccessToken << std::endl;

        // Sets up credentials for EOS Connect login using the retrieved authentication token
        EOS_Connect_Credentials Credentials = {};
        Credentials.ApiVersion = EOS_CONNECT_CREDENTIALS_API_LATEST;
        Credentials.Token = AuthToken->AccessToken;
        Credentials.Type = EOS_EExternalCredentialType::EOS_ECT_EPIC;

        // Configures the EOS Connect login options
        EOS_Connect_LoginOptions ConnectLoginOptions = {};
        ConnectLoginOptions.ApiVersion = EOS_CONNECT_LOGIN_API_LATEST;
        ConnectLoginOptions.Credentials = &Credentials;

        // Initiates the connect login process with EOS Connect
        EOS_Connect_Login(ConnectHandle, &ConnectLoginOptions, instance, OnConnectLoginComplete);

        EOS_Auth_Token_Release(AuthToken);
    }
    else {
        std::cerr << "[ERROR] Failed to copy user auth token. Error: " << EOS_EResult_ToString(Result) << std::endl;
    }
}

// Callback function triggered when the EOS Connect login process completes
void EOSInitialisationManager::OnConnectLoginComplete(const EOS_Connect_LoginCallbackInfo* Data) {
    std::cout << "[OnConnectLoginComplete] Callback received." << std::endl;

    // Retrieves the EOSInitialisationManager instance
    EOSInitialisationManager* instance = static_cast<EOSInitialisationManager*>(Data->ClientData);

    if (!instance) {
        std::cerr << "[ERROR] ClientData is null in OnConnectLoginComplete!" << std::endl;
        return;
    }

    // Checks if the login was successful
    if (Data->ResultCode == EOS_EResult::EOS_Success) {
        instance->LocalUserId = Data->LocalUserId;

        // Validates the retrieved LocalUserId
        if (!instance->LocalUserId || !EOS_ProductUserId_IsValid(instance->LocalUserId)) {
            std::cerr << "[ERROR] LocalUserId is NULL or invalid after successful login!" << std::endl;
            return;
        }

        // Buffer to store the string representation of the LocalUserId
        char UserIdBuffer[EOS_PRODUCTUSERID_MAX_LENGTH + 1] = {};
        int32_t BufferSize = EOS_PRODUCTUSERID_MAX_LENGTH + 1;

        // Converts LocalUserId to a string format
        EOS_EResult res = EOS_ProductUserId_ToString(instance->LocalUserId, UserIdBuffer, &BufferSize);

        // Checks if conversion was successful
        if (res == EOS_EResult::EOS_Success) {
            std::cout << "[OnConnectLoginComplete] LocalUserId: " << UserIdBuffer << std::endl;
        }
        else {
            std::cerr << "[ERROR] Failed to convert LocalUserId to string. Error: "
                << EOS_EResult_ToString(res) << std::endl;
        }

        instance->running = false;

        // Calls the authentication complete callback
        if (instance->authCompleteCallback) {
            instance->authCompleteCallback();
        }
    }

    // If the user is not linked to an EOS Connect account, attempt to create a new user 
    // Deals with authentication problems faced in the lab

    else if (Data->ResultCode == EOS_EResult::EOS_InvalidUser && Data->ContinuanceToken) {
        std::cout << "[OnConnectLoginComplete] User is not linked to EOS Connect. Attempting to create user..." << std::endl;

        // Retrieves the EOS Connect interface handle
        EOS_HConnect ConnectHandle = EOS_Platform_GetConnectInterface(instance->PlatformHandle);

        // Sets up options for creating a new EOS Connect user
        EOS_Connect_CreateUserOptions CreateUserOptions = {};
        CreateUserOptions.ApiVersion = EOS_CONNECT_CREATEUSER_API_LATEST;
        CreateUserOptions.ContinuanceToken = Data->ContinuanceToken;

        // Initiates the user creation process
        EOS_Connect_CreateUser(ConnectHandle, &CreateUserOptions, instance, OnCreateUserComplete);
    }
    else {
        std::cerr << "[ERROR] EOS Connect Login Failed: " << EOS_EResult_ToString(Data->ResultCode) << std::endl;
    }
}

// Callback function triggered when an EOS Connect user creation process completes
void EOSInitialisationManager::OnCreateUserComplete(const EOS_Connect_CreateUserCallbackInfo* Data) {
    std::cout << "[OnCreateUserComplete] Callback received." << std::endl;

    // Retrieves the EOSInitialisationManager instance
    EOSInitialisationManager* instance = static_cast<EOSInitialisationManager*>(Data->ClientData);

    if (!instance) {
        std::cerr << "[ERROR] ClientData is null in OnCreateUserComplete!" << std::endl;
        return;
    }

    // Checks if the user creation was successful
    if (Data->ResultCode == EOS_EResult::EOS_Success) {
        // Stores and validates the retrieved LocalUserId
        instance->LocalUserId = Data->LocalUserId;
        if (!instance->LocalUserId || !EOS_ProductUserId_IsValid(instance->LocalUserId)) {
            std::cerr << "[ERROR] LocalUserId is NULL or invalid after user creation!" << std::endl;
            return;
        }

        // Buffer to store the string representation of the LocalUserId
        char UserIdBuffer[EOS_PRODUCTUSERID_MAX_LENGTH + 1] = {};
        int32_t BufferSize = EOS_PRODUCTUSERID_MAX_LENGTH + 1;

        // Converts LocalUserId to a string format
        EOS_EResult res = EOS_ProductUserId_ToString(instance->LocalUserId, UserIdBuffer, &BufferSize);

        // Checks if conversion was successful
        if (res == EOS_EResult::EOS_Success) {
            std::cout << "[OnCreateUserComplete] New EOS Connect User created successfully. LocalUserId: " << UserIdBuffer << std::endl;
        }
        else {
            std::cerr << "[ERROR] Failed to convert LocalUserId to string. Error: "
                << EOS_EResult_ToString(res) << std::endl;
        }

        instance->running = false;

        // Calls the authentication complete callback if it is set
        if (instance->authCompleteCallback) {
            instance->authCompleteCallback();
        }
    }
    else {
        std::cerr << "[ERROR] Failed to create EOS Connect User: " << EOS_EResult_ToString(Data->ResultCode) << std::endl;
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

#endif