#include "EOSIPDistribution.h"

EOSIPDistribution& EOSIPDistribution::GetInstance() {
    static EOSIPDistribution instance;
    return instance;
}

// Sends a packet containing an IPv4 address to a specified target user using EOS P2P
void EOSIPDistribution::SendPacket(EOS_ProductUserId TargetUserId)
{
    // Retrieves the EOSInitialisationManager singleton instance
    EOSInitialisationManager& eosInitManager = EOSInitialisationManager::GetInstance();

    // Gets the EOS platform handle from the initialisation manager
    EOS_HPlatform PlatformHandle = eosInitManager.GetPlatformHandle();
    std::cout << "[DEBUG] Entering SendPacket()\n";

    // Validates the TargetUserId before proceeding
    if (!EOS_ProductUserId_IsValid(TargetUserId)) {
        std::cerr << "[ERROR] Invalid TargetUserId. Ensure authentication is complete before sending packets.\n";
        return;
    }

    // Configures the socket ID for the P2P connection
    EOS_P2P_SocketId socketId = {};
    socketId.ApiVersion = EOS_P2P_SOCKETID_API_LATEST;

    // Copies the socket name into the structure
    strncpy_s(socketId.SocketName, "TestSocket", sizeof(socketId.SocketName) - 1);

    // Configures the options for sending a P2P packet
    EOS_P2P_SendPacketOptions sendOptions = {};
    sendOptions.ApiVersion = EOS_P2P_SENDPACKET_API_LATEST;
    sendOptions.LocalUserId = TargetUserId;
    sendOptions.RemoteUserId = TargetUserId;
    sendOptions.SocketId = &socketId;
    sendOptions.Channel = CHANNEL;
    sendOptions.bAllowDelayedDelivery = EOS_FALSE;
    sendOptions.Reliability = EOS_EPacketReliability::EOS_PR_ReliableUnordered;
    sendOptions.bDisableAutoAcceptConnection = EOS_FALSE;

    // Defines the IPv4 address to be sent (192.0.0.1 in hex format)
    uint8_t ipAddress[4] = { 0xC0, 0x00, 0x00, 0x01 };

    // Sets the data payload of the packet to the IPv4 address
    sendOptions.DataLengthBytes = sizeof(ipAddress);
    sendOptions.Data = ipAddress;

    // Retrieves the EOS P2P interface handle
    EOS_HP2P P2PHandle = EOS_Platform_GetP2PInterface(PlatformHandle);

    // Ensures the P2P interface handle is valid
    if (!P2PHandle) {
        std::cerr << "[ERROR] P2P Interface handle is NULL!\n";
        return;
    }

    // Sends the P2P packet using the configured options
    EOS_EResult result = EOS_P2P_SendPacket(P2PHandle, &sendOptions);

    // Checks if the packet was sent successfully
    if (result == EOS_EResult::EOS_Success) {
        std::cout << "[P2P] Sent IPv4 address 192.0.0.1 to user successfully!\n";
    }
    else {
        std::cerr << "[ERROR] Failed to send packet: " << EOS_EResult_ToString(result) << "\n";
    }
}

// Receives a packet from any sender and stores the received IP
bool EOSIPDistribution::ReceivePacket(EOS_ProductUserId LocalUserId, std::unordered_map<std::string, std::string>& collectedIPs)
{
    // Retrieves the EOSInitialisationManager singleton instance
    EOSInitialisationManager& eosInitManager = EOSInitialisationManager::GetInstance();

    // Gets the EOS platform handle
    EOS_HPlatform PlatformHandle = eosInitManager.GetPlatformHandle();
    std::cout << "[DEBUG] Entering ReceivePacket()\n";

    // Retrieves the EOS P2P interface handle
    EOS_HP2P P2PHandle = EOS_Platform_GetP2PInterface(PlatformHandle);

    // Ensures the P2P interface handle is valid
    if (!P2PHandle) {
        std::cerr << "[ERROR] P2P Interface handle is NULL!\n";
        return false;
    }

    // Validates the LocalUserId before proceeding
    if (!EOS_ProductUserId_IsValid(LocalUserId)) {
        std::cerr << "[ERROR] LocalUserId is invalid. Ensure authentication is complete before receiving packets.\n";
        return false;
    }

    // Variable to store the size of the next packet
    uint32_t NextPacketSize = 0;

    // Sets up options to check for the next received packet size
    EOS_P2P_GetNextReceivedPacketSizeOptions sizeOptions = {};
    sizeOptions.ApiVersion = EOS_P2P_GETNEXTRECEIVEDPACKETSIZE_API_LATEST;
    sizeOptions.LocalUserId = LocalUserId;

    // Queries the size of the next available packet
    EOS_EResult sizeResult = EOS_P2P_GetNextReceivedPacketSize(P2PHandle, &sizeOptions, &NextPacketSize);

    // Checks if there are no packets available
    if (sizeResult != EOS_EResult::EOS_Success || NextPacketSize == 0) {
        std::cout << "[P2P] No new packets available.\n";
        return false;
    }

    // Sets up options to receive a packet
    EOS_P2P_ReceivePacketOptions receiveOptions = {};
    receiveOptions.ApiVersion = EOS_P2P_RECEIVEPACKET_API_LATEST;
    receiveOptions.LocalUserId = LocalUserId;
    receiveOptions.MaxDataSizeBytes = PACKET_SIZE;

    EOS_ProductUserId SenderId = nullptr;
    EOS_P2P_SocketId socketId = {};

    // Assigns the socket name
    strncpy_s(socketId.SocketName, "TestSocket", sizeof(socketId.SocketName));
    socketId.SocketName[sizeof(socketId.SocketName) - 1] = '\0';

    uint8_t Channel = CHANNEL;
    uint32_t DataSize = 0;
    uint8_t DataBuffer[PACKET_SIZE] = {};

    // Receives the packet from the EOS P2P interface
    EOS_EResult result = EOS_P2P_ReceivePacket(
        P2PHandle, &receiveOptions, &SenderId, &socketId, &Channel, DataBuffer, &DataSize);

    // Checks if the packet was received successfully and contains a valid IPv4 address
    if (result == EOS_EResult::EOS_Success && DataSize == 4) {
        char senderUserIdString[EOS_PRODUCTUSERID_MAX_LENGTH + 1] = {};
        int32_t bufferSize = sizeof(senderUserIdString);

        // Constructs the received IP address from the received byte data
        if (EOS_ProductUserId_ToString(SenderId, senderUserIdString, &bufferSize) == EOS_EResult::EOS_Success) {
            std::string senderIdStr(senderUserIdString);
            std::string ipReceived =
                std::to_string(DataBuffer[0]) + "." +
                std::to_string(DataBuffer[1]) + "." +
                std::to_string(DataBuffer[2]) + "." +
                std::to_string(DataBuffer[3]);

            // Stores the IP address in collectedIPs if the sender is not already recorded
            if (collectedIPs.find(senderIdStr) == collectedIPs.end()) {
                collectedIPs[senderIdStr] = ipReceived;
                std::cout << "[P2P] Received IP from " << senderIdStr << ": " << ipReceived << "\n";
                return true;
            }
        }
        else {
            std::cerr << "[ERROR] Failed to convert SenderId to string.\n";
        }
    }
    else {
        std::cerr << "[ERROR] Failed to receive packet: " << EOS_EResult_ToString(result) << "\n";
    }

    // Returns false if no valid packet was received
    return false;
}
