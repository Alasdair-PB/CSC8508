#include "EOSIPDistribution.h"

EOSIPDistribution& EOSIPDistribution::GetInstance() {
    static EOSIPDistribution instance;
    return instance;
}

void EOSIPDistribution::SendPacket(EOS_ProductUserId TargetUserId)
{
    EOSInitialisationManager& eosInitManager = EOSInitialisationManager::GetInstance();
    EOS_HPlatform PlatformHandle = eosInitManager.GetPlatformHandle();
    std::cout << "[DEBUG] Entering SendPacket()\n";

    if (!EOS_ProductUserId_IsValid(TargetUserId)) {
        std::cerr << "[ERROR] Invalid TargetUserId. Ensure authentication is complete before sending packets.\n";
        return;
    }

    EOS_P2P_SocketId socketId = {};
    socketId.ApiVersion = EOS_P2P_SOCKETID_API_LATEST;
    strncpy_s(socketId.SocketName, "TestSocket", sizeof(socketId.SocketName) - 1);

    EOS_P2P_SendPacketOptions sendOptions = {};
    sendOptions.ApiVersion = EOS_P2P_SENDPACKET_API_LATEST;
    sendOptions.LocalUserId = TargetUserId;
    sendOptions.RemoteUserId = TargetUserId;
    sendOptions.SocketId = &socketId;
    sendOptions.Channel = CHANNEL;
    sendOptions.bAllowDelayedDelivery = EOS_FALSE;
    sendOptions.Reliability = EOS_EPacketReliability::EOS_PR_ReliableUnordered;
    sendOptions.bDisableAutoAcceptConnection = EOS_FALSE;

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
        std::cout << "[P2P] Sent IPv4 address 192.0.0.1 to user successfully!\n";
        ReceivePacket(TargetUserId);
    }
    else {
        std::cerr << "[ERROR] Failed to send packet: " << EOS_EResult_ToString(result) << "\n";
    }
}

void EOSIPDistribution::ReceivePacket(EOS_ProductUserId TargetUserId)
{
    EOSInitialisationManager& eosInitManager = EOSInitialisationManager::GetInstance();
    EOS_HPlatform PlatformHandle = eosInitManager.GetPlatformHandle();
    std::cout << "[DEBUG] Entering ReceivePacket()\n";

    EOS_HP2P P2PHandle = EOS_Platform_GetP2PInterface(PlatformHandle);
    if (!P2PHandle) {
        std::cerr << "[ERROR] P2P Interface handle is NULL!\n";
        return;
    }

    if (!EOS_ProductUserId_IsValid(TargetUserId)) {
        std::cerr << "[ERROR] TargetUserId is invalid. Ensure authentication is complete before receiving packets.\n";
        return;
    }

    uint32_t NextPacketSize = 0;
    EOS_P2P_GetNextReceivedPacketSizeOptions sizeOptions = {};
    sizeOptions.ApiVersion = EOS_P2P_GETNEXTRECEIVEDPACKETSIZE_API_LATEST;
    sizeOptions.LocalUserId = TargetUserId;

    EOS_EResult sizeResult = EOS_P2P_GetNextReceivedPacketSize(P2PHandle, &sizeOptions, &NextPacketSize);
    if (sizeResult != EOS_EResult::EOS_Success || NextPacketSize == 0) {
        std::cout << "[P2P] No new packets available.\n";
        return;
    }

    EOS_P2P_ReceivePacketOptions receiveOptions = {};
    receiveOptions.ApiVersion = EOS_P2P_RECEIVEPACKET_API_LATEST;
    receiveOptions.LocalUserId = TargetUserId;
    receiveOptions.MaxDataSizeBytes = PACKET_SIZE;

    EOS_ProductUserId SenderId = nullptr;
    EOS_P2P_SocketId socketId = {};
    strncpy_s(socketId.SocketName, "TestSocket", sizeof(socketId.SocketName));
    socketId.SocketName[sizeof(socketId.SocketName) - 1] = '\0';

    uint8_t Channel = CHANNEL;
    uint32_t DataSize = 0;
    uint8_t DataBuffer[PACKET_SIZE] = {};

    EOS_EResult result = EOS_P2P_ReceivePacket(
        P2PHandle, &receiveOptions, &SenderId, &socketId, &Channel, DataBuffer, &DataSize);

    if (result == EOS_EResult::EOS_Success) {
        std::cout << "[DEBUG] Packet received successfully!\n";
        if (DataSize == 4) {
            std::cout << "[P2P] Received Packet (IPv4 Address Representation): ";
            std::cout << static_cast<int>(DataBuffer[0]) << "."
                << static_cast<int>(DataBuffer[1]) << "."
                << static_cast<int>(DataBuffer[2]) << "."
                << static_cast<int>(DataBuffer[3]) << "\n";
        }
    }
    else {
        std::cerr << "[ERROR] Failed to receive packet: " << EOS_EResult_ToString(result) << "\n";
    }
}
