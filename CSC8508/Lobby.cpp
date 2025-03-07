#include "lobby.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <eos_p2p.h>

EOS_HPlatform PlatformHandle = nullptr;
EOS_HLobbyDetails LobbyDetailsHandle;
EOS_ProductUserId LocalUserId = nullptr;
EOS_HLobbySearch LobbySearchHandle = nullptr;

EOS_HLobby LobbyHandle = {};

// Define constants
#define CHANNEL 0
#define PACKET_SIZE 256

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
            //LeaveLobby();
            CreateLobby();
            //CreateLobbySearch();
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

    LobbyHandle = EOS_Platform_GetLobbyInterface(PlatformHandle);

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
}


void OnLobbyCreated(const EOS_Lobby_CreateLobbyCallbackInfo* Data) {
    std::cout << "[OnLobbyCreated] Callback received." << std::endl;

    if (Data->ResultCode == EOS_EResult::EOS_Success) {
        std::cout << "[OnLobbyCreated] Lobby created successfully! Lobby ID: " << Data->LobbyId << ". You are now the owner!" << std::endl;

        EOS_HLobbyModification LobbyModificationHandle = nullptr;
        EOS_Lobby_UpdateLobbyModificationOptions ModOptions = {};
        ModOptions.ApiVersion = EOS_LOBBY_UPDATELOBBYMODIFICATION_API_LATEST;
        ModOptions.LobbyId = Data->LobbyId;
        ModOptions.LocalUserId = LocalUserId;

        EOS_EResult Result = EOS_Lobby_UpdateLobbyModification(LobbyHandle, &ModOptions, &LobbyModificationHandle);
        if (Result == EOS_EResult::EOS_Success) {
            std::cout << "[OnLobbyCreated] Lobby Modification Handle successfully created." << std::endl;

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

            // Apply the modifications
            EOS_Lobby_UpdateLobbyOptions UpdateOptions = {};
            UpdateOptions.ApiVersion = EOS_LOBBY_UPDATELOBBY_API_LATEST;
            UpdateOptions.LobbyModificationHandle = LobbyModificationHandle;

            EOS_Lobby_UpdateLobby(LobbyHandle, &UpdateOptions, nullptr, OnLobbyUpdated);

            // Release the modification handle
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

void CreateLobbySearch() {
    std::cout << "[CreateLobbySearch] Attempting to create a lobby search..." << std::endl;

    if (!LocalUserId) {
        std::cerr << "[ERROR] LocalUserId is NULL. Authentication must be complete before creating a lobby search." << std::endl;
        return;
    }

    if (!PlatformHandle) {
        std::cerr << "[ERROR] PlatformHandle is NULL. Ensure the EOS platform is initialized." << std::endl;
        return;
    }

    EOS_HLobby LobbyHandle = EOS_Platform_GetLobbyInterface(PlatformHandle);

    if (!LobbyHandle) {
        std::cerr << "[ERROR] LobbyHandle is NULL. Lobby interface might not be initialized." << std::endl;
        return;
    }

    // Initialize the lobby search options
    EOS_Lobby_CreateLobbySearchOptions SearchOptions = {};
    SearchOptions.ApiVersion = EOS_LOBBY_CREATELOBBYSEARCH_API_LATEST;
    SearchOptions.MaxResults = 50;  // Max number of lobbies to return

    // Create the lobby search handle
    
    EOS_EResult Result = EOS_Lobby_CreateLobbySearch(LobbyHandle, &SearchOptions, &LobbySearchHandle);

    if (Result == EOS_EResult::EOS_Success) {
        std::cout << "[CreateLobbySearch] Lobby search created successfully." << std::endl;

        // Example attribute: Searching for lobbies with "GameMode" set to "Deathmatch"
        EOS_LobbySearch_SetParameterOptions SetParameterOptions = {};
        SetParameterOptions.ApiVersion = EOS_LOBBYSEARCH_SETPARAMETER_API_LATEST;

        EOS_Lobby_AttributeData AttributeData = {};
        AttributeData.ApiVersion = EOS_LOBBY_ATTRIBUTEDATA_API_LATEST;
        AttributeData.ValueType = EOS_ELobbyAttributeType::EOS_AT_STRING;

        AttributeData.Key = "LOBBYSERVICEATTRIBUTE1"; // The key of the attribute
        AttributeData.Value.AsUtf8 = "SEARCHKEYWORDS"; // Assign value to the correct field

        SetParameterOptions.Parameter = &AttributeData;
        SetParameterOptions.ComparisonOp = EOS_EComparisonOp::EOS_CO_EQUAL; // Exact match for "GameMode" attribute

        // Apply the filter to the search
        Result = EOS_LobbySearch_SetParameter(LobbySearchHandle, &SetParameterOptions);

        if (Result != EOS_EResult::EOS_Success) {
            std::cerr << "[ERROR] Failed to set search parameter. Error: " << EOS_EResult_ToString(Result) << std::endl;
            return;
        }
        else
        {
            std::cerr << "[ERROR] Parameter set! " << std::endl;
        }

        std::cout << "[CreateLobbySearch] Filter applied: Searching..." << std::endl;

        // Set up the FindOptions with LocalUserId
        EOS_LobbySearch_FindOptions FindOptions = {};
        FindOptions.ApiVersion = EOS_LOBBYSEARCH_FIND_API_LATEST;
        FindOptions.LocalUserId = LocalUserId;  // Use LocalUserId for the search

        // Call EOS_LobbySearch_Find to execute the search
        EOS_LobbySearch_Find(LobbySearchHandle, &FindOptions, nullptr, OnFindLobbiesComplete);
    }
    else {
        std::cerr << "[ERROR] Failed to create lobby search. Error: " << EOS_EResult_ToString(Result) << std::endl;
    }
}

void OnFindLobbiesComplete(const EOS_LobbySearch_FindCallbackInfo* Data) {
    if (Data->ResultCode == EOS_EResult::EOS_Success) {

        int32_t numResults = 0;
        EOS_LobbySearch_GetSearchResultCountOptions countOptions = {};
        countOptions.ApiVersion = EOS_LOBBYSEARCH_GETSEARCHRESULTCOUNT_API_LATEST;

        numResults = EOS_LobbySearch_GetSearchResultCount(LobbySearchHandle, &countOptions);

        std::cerr << "[OnFindLobbiesComplete] Found " << numResults << " lobbies.\n";

        if (numResults == 0) {
            std::cerr << "[DEBUG] No lobbies found. Check attributes and permissions.\n";
            return;
        }

        LobbyDetailsHandle = nullptr;

        EOS_LobbySearch_CopySearchResultByIndexOptions CopyOptions = {};
        CopyOptions.ApiVersion = EOS_LOBBYSEARCH_COPYSEARCHRESULTBYINDEX_API_LATEST;
        CopyOptions.LobbyIndex = 1;

        EOS_EResult Result = EOS_LobbySearch_CopySearchResultByIndex(LobbySearchHandle, &CopyOptions, &LobbyDetailsHandle);
        std::this_thread::sleep_for(std::chrono::seconds(2));
        if (Result == EOS_EResult::EOS_Success) {
            std::cerr << "[LobbyData] Lobby Details Retrieved\n";

            // Now retrieve the lobby details
            EOS_LobbyDetails_Info* LobbyInfo = nullptr;
            EOS_LobbyDetails_CopyInfoOptions InfoOptions = {};
            InfoOptions.ApiVersion = EOS_LOBBYDETAILS_COPYINFO_API_LATEST;

            EOS_EResult InfoResult = EOS_LobbyDetails_CopyInfo(LobbyDetailsHandle, &InfoOptions, &LobbyInfo);

            if (InfoResult == EOS_EResult::EOS_Success && LobbyInfo) {
                std::cerr << "[LobbyInfo] Lobby ID: " << LobbyInfo->LobbyId << "\n";
                std::cerr << "[LobbyInfo] Max Players: " << LobbyInfo->MaxMembers << "\n";
                std::cerr << "[LobbyInfo] Owner ID: " << LobbyInfo->LobbyOwnerUserId << "\n";

                // LeaveLobby();
                // JoinLobby();

                // Free the memory allocated for the lobby info structure
                EOS_LobbyDetails_Info_Release(LobbyInfo);
            }
            else {
                std::cerr << "[ERROR] Failed to retrieve lobby info. Error: " << EOS_EResult_ToString(InfoResult) << std::endl;
            }
        }
        else {
            std::cerr << "[LobbyData] Failed to fetch lobby data. Error: " << EOS_EResult_ToString(Result) << std::endl;
        }
    }
    else {
        std::cerr << "[OnFindLobbiesComplete] Lobby search failed. Error: " << EOS_EResult_ToString(Data->ResultCode) << std::endl;
    }
}

// Function to join a lobby using a given LobbyDetailsHandle
void JoinLobby() {
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
    JoinOptions.LocalUserId = LocalUserId;  // Ensure LocalUserId is correctly set
    JoinOptions.LocalRTCOptions = NULL;
    JoinOptions.bPresenceEnabled = EOS_TRUE;  // Enable presence if needed
    JoinOptions.bCrossplayOptOut = EOS_FALSE;
    JoinOptions.RTCRoomJoinActionType = EOS_ELobbyRTCRoomJoinActionType::EOS_LRRJAT_AutomaticJoin;

    EOS_Lobby_JoinLobby(LobbyHandle, &JoinOptions, nullptr, OnJoinLobbyComplete);
}

// Callback function for when joining a lobby completes
void OnJoinLobbyComplete(const EOS_Lobby_JoinLobbyCallbackInfo* Data) {
    if (Data->ResultCode == EOS_EResult::EOS_Success) {
    }
    else {
        std::cerr << "[ERROR] Failed to join lobby. Error: " << EOS_EResult_ToString(Data->ResultCode) << std::endl;
    }
}

// Callback after updating the lobby
void OnLobbyUpdated(const EOS_Lobby_UpdateLobbyCallbackInfo* Data) {
    if (Data->ResultCode == EOS_EResult::EOS_Success) {
        std::cout << "[OnLobbyUpdated] Successfully updated lobby attributes.\n";
        SendPacketToSelf();  // Accept the connection before sending packets
    }
    else {
        std::cerr << "[ERROR] Failed to update lobby. Error: " << EOS_EResult_ToString(Data->ResultCode) << std::endl;
    }
}

// Function to leave a lobby using a given LobbyId
void LeaveLobby() {

    LobbyHandle = EOS_Platform_GetLobbyInterface(PlatformHandle);

    EOS_Lobby_LeaveLobbyOptions LeaveOptions = {};
    LeaveOptions.ApiVersion = EOS_LOBBY_LEAVELOBBY_API_LATEST;
    LeaveOptions.LocalUserId = LocalUserId; // Ensure LocalUserId is correctly set
    LeaveOptions.LobbyId = "8b04b581a080487c98ee5ee27338e65b"; //Change this

    EOS_Lobby_LeaveLobby(LobbyHandle, &LeaveOptions, nullptr, OnLeaveLobbyComplete);
}

// Callback function for when leaving a lobby completes
void OnLeaveLobbyComplete(const EOS_Lobby_LeaveLobbyCallbackInfo* Data) {
    if (Data->ResultCode == EOS_EResult::EOS_Success) {
        std::cerr << "[OnLeaveLobbyComplete] Successfully left lobby: " << Data->LobbyId << "\n";
    }
    else {
        std::cerr << "[ERROR] Failed to leave lobby. Error: " << EOS_EResult_ToString(Data->ResultCode) << std::endl;
    }
}

void SendPacketToSelf()
{
    std::cout << "[DEBUG] Entering SendPacketToSelf()\n";

    if (!EOS_ProductUserId_IsValid(LocalUserId)) {
        std::cerr << "[ERROR] Invalid LocalUserId. Ensure authentication is complete before sending packets.\n";
        return;
    }

    EOS_P2P_SocketId socketId = {};
    socketId.ApiVersion = EOS_P2P_SOCKETID_API_LATEST;
    strncpy_s(socketId.SocketName, "TestSocket", sizeof(socketId.SocketName) - 1);

    EOS_P2P_SendPacketOptions sendOptions = {};
    sendOptions.ApiVersion = EOS_P2P_SENDPACKET_API_LATEST;
    sendOptions.LocalUserId = LocalUserId;
    sendOptions.RemoteUserId = LocalUserId;
    sendOptions.SocketId = &socketId;
    sendOptions.Channel = CHANNEL;
    sendOptions.bAllowDelayedDelivery = EOS_FALSE;
    sendOptions.Reliability = EOS_EPacketReliability::EOS_PR_ReliableUnordered;
    sendOptions.bDisableAutoAcceptConnection = EOS_FALSE;

    // IPv4 address 192.0.0.1 as 4-byte data
    uint8_t ipAddress[4] = { 0xC0, 0x00, 0x00, 0x01 };
    sendOptions.DataLengthBytes = sizeof(ipAddress);
    sendOptions.Data = ipAddress;

    EOS_HP2P P2PHandle = EOS_Platform_GetP2PInterface(PlatformHandle);
    if (!P2PHandle) {
        std::cerr << "[ERROR] P2P Interface handle is NULL!\n";
        return;
    }

    EOS_EResult result = EOS_P2P_SendPacket(P2PHandle, &sendOptions);
    if (result == EOS_EResult::EOS_Success) {
        std::cout << "[P2P] Sent IPv4 address 192.0.0.1 to self successfully!\n";
        ReceivePacket();
    }
    else {
        std::cerr << "[ERROR] Failed to send packet to self: " << EOS_EResult_ToString(result) << "\n";
    }
}

void ReceivePacket()
{
    std::cout << "[DEBUG] Entering ReceivePacket()\n";

    EOS_HP2P P2PHandle = EOS_Platform_GetP2PInterface(PlatformHandle);
    if (!P2PHandle) {
        std::cerr << "[ERROR] P2P Interface handle is NULL!\n";
        return;
    }

    if (!LocalUserId || !EOS_ProductUserId_IsValid(LocalUserId)) {
        std::cerr << "[ERROR] LocalUserId is NULL or invalid. Ensure authentication is complete before receiving packets.\n";
        return;
    }

    uint32_t NextPacketSize = 0;
    EOS_P2P_GetNextReceivedPacketSizeOptions sizeOptions = {};
    sizeOptions.ApiVersion = EOS_P2P_GETNEXTRECEIVEDPACKETSIZE_API_LATEST;
    sizeOptions.LocalUserId = LocalUserId;

    std::cout << "[DEBUG] Checking next packet size...\n";
    EOS_EResult sizeResult = EOS_P2P_GetNextReceivedPacketSize(P2PHandle, &sizeOptions, &NextPacketSize);

    if (sizeResult != EOS_EResult::EOS_Success || NextPacketSize == 0)
    {
        std::cout << "[P2P] No new packets available.\n";
        return;
    }

    std::cout << "[DEBUG] Next packet size: " << NextPacketSize << " bytes\n";

    if (NextPacketSize > PACKET_SIZE)
    {
        std::cerr << "[ERROR] Incoming packet size (" << NextPacketSize
            << " bytes) exceeds buffer size (" << PACKET_SIZE
            << " bytes). Possible corruption.\n";
        return;
    }

    EOS_P2P_ReceivePacketOptions receiveOptions = {};
    receiveOptions.ApiVersion = EOS_P2P_RECEIVEPACKET_API_LATEST;
    receiveOptions.LocalUserId = LocalUserId;
    receiveOptions.MaxDataSizeBytes = PACKET_SIZE;

    EOS_ProductUserId SenderId = nullptr;
    EOS_P2P_SocketId socketId = {};

    strncpy_s(socketId.SocketName, "TestSocket", sizeof(socketId.SocketName));
    socketId.SocketName[sizeof(socketId.SocketName) - 1] = '\0';

    std::cout << "[DEBUG] Socket ID set: " << socketId.SocketName << "\n";

    uint8_t Channel = CHANNEL;
    uint32_t DataSize = 0;
    uint8_t DataBuffer[PACKET_SIZE] = {};  // Use byte buffer to avoid alignment issues

    std::cout << "[DEBUG] Calling EOS_P2P_ReceivePacket()...\n";

    EOS_EResult result = EOS_P2P_ReceivePacket(
        P2PHandle,
        &receiveOptions,
        &SenderId,
        &socketId,
        &Channel,
        DataBuffer,
        &DataSize
    );

    std::cout << "[DEBUG] EOS_P2P_ReceivePacket() returned: " << static_cast<int>(result) << "\n";
    std::cout << "[DEBUG] DataSize after receive: " << DataSize << " bytes\n";

    if (result == EOS_EResult::EOS_Success)
    {
        if (DataSize > PACKET_SIZE)
        {
            std::cerr << "[ERROR] Received data exceeds buffer size! Possible corruption.\n";
            return;
        }

        char SenderBuffer[EOS_PRODUCTUSERID_MAX_LENGTH + 1] = {};
        int32_t BufferSize = sizeof(SenderBuffer);
        EOS_ProductUserId_ToString(SenderId, SenderBuffer, &BufferSize);

        std::cout << "[DEBUG] Packet received successfully!\n";
        std::cout << "[DEBUG] Sender User ID: " << SenderBuffer << "\n";
        std::cout << "[DEBUG] Socket ID: " << socketId.SocketName << "\n";
        std::cout << "[DEBUG] Channel: " << static_cast<int>(Channel) << "\n";
        std::cout << "[DEBUG] DataSize: " << DataSize << " bytes\n";

        std::cout << "[P2P] Received Packet (hex dump):\n";
        for (uint32_t i = 0; i < DataSize; i++) {
            printf("%02X ", DataBuffer[i]);  // Print each byte as hex
            if ((i + 1) % 16 == 0) printf("\n");  // New line every 16 bytes
        }
        std::cout << "\n";

        // If DataSize is 4, assume it's an IPv4 address
        if (DataSize == 4)
        {
            std::cout << "[P2P] Received Packet (IPv4 Address Representation): ";
            std::cout << static_cast<int>(DataBuffer[0]) << "."
                << static_cast<int>(DataBuffer[1]) << "."
                << static_cast<int>(DataBuffer[2]) << "."
                << static_cast<int>(DataBuffer[3]) << "\n";
        }
        else
        {
            std::cout << "[P2P] Received Packet (ASCII representation):\n";
            for (uint32_t i = 0; i < DataSize; i++) {
                if (isprint(DataBuffer[i])) {
                    std::cout << static_cast<char>(DataBuffer[i]);  // Print readable characters
                }
                else {
                    std::cout << ".";  // Print a dot for non-printable characters
                }
            }
            std::cout << "\n";
        }
    }
    else if (result == EOS_EResult::EOS_NotFound)
    {
        std::cout << "[P2P] No new packets received.\n";
    }
    else
    {
        std::cerr << "[ERROR] Failed to receive packet: " << EOS_EResult_ToString(result)
            << " (Code: " << static_cast<int>(result) << ")\n";
    }

    std::cout << "[DEBUG] Exiting ReceivePacket()\n";
}
