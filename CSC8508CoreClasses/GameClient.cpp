#include "GameClient.h"
#include "./enet/enet.h"

using namespace NCL;
using namespace CSC8508;

GameClient::GameClient()	{
	netHandle = enet_host_create(nullptr, 1, 1, 0, 0);
}

GameClient::~GameClient()	{
	enet_host_destroy(netHandle);
}

bool GameClient::Connect(uint8_t a, uint8_t b, uint8_t c, uint8_t d, int portNum) 
{
	ENetAddress address;
	address.port = portNum;
	address.host = (d << 24) | (c << 16) | (b << 8) | (a);
	netPeer = enet_host_connect(netHandle, &address, 2, 0);
	return netPeer != nullptr;
}

void GameClient::ReceivePacket(int type, GamePacket* payload, int source)
{
	if (payload->type == Player_Connected) 
	{
		SetClientId* recievedPacket = (SetClientId*)payload;
		peerID = recievedPacket->clientPeerId;
		AcknowledgePacket* ackPacket = new AcknowledgePacket(peerID);
		std::cout << "Sending ackowledge package" << std::endl;
		SendPacket(*ackPacket);
		delete ackPacket;
	}
}

void GameClient::UpdateClient() 
{
	if (netHandle == nullptr) 
		return;

	ENetEvent event;
	while (enet_host_service(netHandle, &event, 0) > 0) {
		if (event.type == ENET_EVENT_TYPE_CONNECT) 
			std::cout << "Connected to server!" << std::endl;
		else if (event.type == ENET_EVENT_TYPE_RECEIVE) {
			std::cout << "Client: Packet received..." << std::endl;
			GamePacket* packet = (GamePacket*)event.packet->data;
			ProcessPacket(packet);
		}
		enet_packet_destroy(event.packet);
	}
}

// Modify for different channel when not local?
void GameClient::SendPacket(GamePacket&  payload) 
{
	std::cout << "Sending package" << std::endl;
	ENetPacket* dataPacket = enet_packet_create(&payload, payload.GetTotalSize(), 0);
	enet_peer_send(netPeer, 0, dataPacket);
}
