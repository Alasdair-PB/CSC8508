#pragma once
#include "Event.h"

//#include "./enet/enet.h"
struct _ENetHost;
struct _ENetPeer;
struct _ENetEvent;

enum BasicNetworkMessages {
	None,
	Message,
	Spawn_Object,
	Delta_State,	
	Full_State,		
	Received_State,
	Player_Connected,
	Player_Disconnected,
	Component_Event,
	Shutdown
};

struct GamePacket {
	short size;
	short type;

	GamePacket() {
		type = BasicNetworkMessages::None;
		size = 0;
	}

	GamePacket(short type) : GamePacket() {
		this->type = type;
	}

	int GetTotalSize() {
		return sizeof(GamePacket) + size;
	}
};

struct SetClientId : public GamePacket {
	int clientPeerId;

	SetClientId() {
		size = sizeof(SetClientId) - sizeof(GamePacket);
		type = Player_Connected;
	}
};

struct AcknowledgePacket : public GamePacket {
	int stateID;

	AcknowledgePacket(int stateID) {
		type = Received_State; 
		size = sizeof(AcknowledgePacket); 
		this->stateID = stateID;
	}

	int GetStateID() const {
		return stateID;
	}
};

class ClientConnectedEvent : Event {
public:
	ClientConnectedEvent(int clientId) { this->clientId = clientId; }
	int GetClientId() { return clientId;  }
protected:
	int clientId;
};

class PacketReceiver {
public:
	virtual void ReceivePacket(int type, GamePacket* payload, int source = -1) = 0;
};

class NetworkBase	{
public:
	static void Initialise();
	static void Destroy();

	static int GetDefaultPort() {
		return 1234;
	}

	void RegisterPacketHandler(int msgID, PacketReceiver* receiver) {
		packetHandlers.insert(std::make_pair(msgID, receiver));
	}

	void SetPeerId(int newId) { peerID = newId; }
	int GetPeerId() { return peerID; }


protected:
	NetworkBase();
	~NetworkBase();

	int peerID = -1;
	bool ProcessPacket(GamePacket* p, int peerID = -1);

	typedef std::multimap<int, PacketReceiver*>::const_iterator PacketHandlerIterator;

	bool GetPacketHandlers(int msgID, PacketHandlerIterator& first, PacketHandlerIterator& last) const {
		auto range = packetHandlers.equal_range(msgID);

		if (range.first == packetHandlers.end()) {
			return false; //no handlers for this message type!
		}
		first	= range.first;
		last	= range.second;
		return true;
	}

	_ENetHost* netHandle;

	std::multimap<int, PacketReceiver*> packetHandlers;
};