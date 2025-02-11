#include "NetworkedGame.h"
#include "NetworkPlayer.h"
#include "NetworkObject.h"
#include "GameServer.h"
#include "GameClient.h"
#include "RenderObject.h"
#include "INetworkComponent.h"

#include "EventManager.h"


#define COLLISION_MSG 30

struct MessagePacket : public GamePacket {
	short playerID;
	short messageID;

	MessagePacket() {
		type = Message;
		size = sizeof(short) * 2;
	}
};


struct SpawnPacket : public GamePacket {

	short ownerId;
	short objectId;
	// add a prefab reference in the future

	SpawnPacket() {
		type = Spawn_Object;
		size = sizeof(short) * 2;
	}
};

void NetworkedGame::StartClientCallBack() { StartAsClient(127, 0, 0, 1); }
void NetworkedGame::StartServerCallBack() { StartAsServer(); }
void NetworkedGame::StartOfflineCallBack() { TutorialGame::AddPlayerToWorld(Vector3(90, 22, -50)); }

NetworkedGame::NetworkedGame()	{
	EventManager::RegisterListener<NetworkEvent>(this);
	EventManager::RegisterListener<ClientConnectedEvent>(this);

	thisServer = nullptr;
	thisClient = nullptr;

	mainMenu = new MainMenu([&](bool state) -> void { this->SetPause(state); },
		[&]() -> void { this->StartClientCallBack(); },
		[&]() -> void { this->StartServerCallBack(); },
		[&]() -> void { this->StartOfflineCallBack();});

	NetworkBase::Initialise();
	timeToNextPacket  = 0.0f;
	packetsToSnapshot = 0;
	playerStates = std::vector<int>();
}

NetworkedGame::~NetworkedGame()	{
	delete thisServer;
	delete thisClient;
}

void NetworkedGame::StartAsServer() 
{
	thisServer = new GameServer(NetworkBase::GetDefaultPort(), 4);
	thisServer->RegisterPacketHandler(Received_State, this);
	thisServer->RegisterPacketHandler(Spawn_Object, this);
	thisServer->RegisterPacketHandler(Component_Event, this);

	thisServer->RegisterPacketHandler(Delta_State, this);
	thisServer->RegisterPacketHandler(Full_State, this);

	SpawnPlayerServer(thisServer->GetPeerId(), Prefab::Player);
}

void NetworkedGame::StartAsClient(char a, char b, char c, char d) 
{
	thisClient = new GameClient();
	thisClient->Connect(a, b, c, d, NetworkBase::GetDefaultPort());

	thisClient->RegisterPacketHandler(Delta_State, this);
	thisClient->RegisterPacketHandler(Full_State, this);
	thisClient->RegisterPacketHandler(Component_Event, this);

	thisClient->RegisterPacketHandler(Player_Connected, this);
	thisClient->RegisterPacketHandler(Player_Disconnected, this);

	thisClient->RegisterPacketHandler(Spawn_Object, this);
}

void NetworkedGame::OnEvent(ClientConnectedEvent* e) 
{
	int id = e->GetClientId();
	SendSpawnPacketsOnClientConnect(id);	
	SpawnPlayerServer(id, Prefab::Player);
}

void NetworkedGame::OnEvent(NetworkEvent* e)
{

	std::cout << "Sending Input packet Data" << std::endl;

	auto dataPacket = e->eventData;
	if (thisServer) {
		for (const auto& player : thisServer->playerPeers)
		{
			int playerID = player.first;
			thisServer->SendPacketToPeer(dataPacket, playerID);
		}
	}
	else 
		thisClient->SendPacket(*dataPacket);
}

void NetworkedGame::UpdateGame(float dt) 
{
	timeToNextPacket -= dt;
	if (timeToNextPacket < 0) {
		if (thisServer) 
			UpdateAsServer(dt);
		else if (thisClient) 
			UpdateAsClient(dt);
		timeToNextPacket += 1.0f / 20.0f; //20hz server/client update
	}
	TutorialGame::UpdateGame(dt);
}

void NetworkedGame::UpdateAsServer(float dt)
{
	packetsToSnapshot--;
	if (packetsToSnapshot < 0) 
	{
		BroadcastSnapshot(false);
		packetsToSnapshot = 5;
	}
	else 
		BroadcastSnapshot(true);
	thisServer->UpdateServer();
}

void NetworkedGame::UpdateAsClient(float dt) 
{
	packetsToSnapshot--;
	if (packetsToSnapshot < 0)
	{
		BroadcastOwnedObjects(false);
		packetsToSnapshot = 5;	
	}
	else 
		BroadcastOwnedObjects(true);
	thisClient->UpdateClient();
}

// Could be reworked if the client is able to learn their owning Id
// Client send all objects in their world not owned by server
void NetworkedGame::BroadcastOwnedObjects(bool deltaFrame) 
{
	/*for (GameObject* i : ownedObjects) {
		NetworkObject* o = i->GetNetworkObject();
		if (!o)
			continue;

		//if (thisClient->GetPeerId() != o->GetOwnerID())
		//	continue;
		GamePacket* newPacket = new GamePacket();
		std::cout << "sending packet to server: " << o->GetObjectID() << std::endl;

		if (o->WritePacket(&newPacket, deltaFrame, o->GetLatestNetworkState().stateID))
			thisClient->SendPacket(*newPacket);
		delete newPacket;
	}*/
}

// Server goes through each object in their world and sends delta or full packets for each. 
void NetworkedGame::BroadcastSnapshot(bool deltaFrame) 
{
	/*for (const auto& player : thisServer->playerPeers)
	{	
		int playerID = player.first;
		std::vector<INetworkComponent*>::const_iterator first, last;
		world->GetINetIterators(first, last);

		for (auto i = first; i != last; ++i) 
		{
			if ((*i)->GetOwnerID() == playerID)
				continue;

			auto packets = (*i)->WritePacket(deltaFrame, (*i)->GetLatestNetworkState().stateID);
			for (int pck =0; pck < packets.size(); pck++)
			{
				thisServer->SendPacketToPeer(packets[pck], playerID);
				std::cout << "sending packet to peer: " << playerID<< ", " << (*i)->GetObjectID() << std::endl;
				delete packets[pck];
			}				
		}
	}*/
}

void NetworkedGame::UpdateMinimumState() 
{
	/*int minID = INT_MAX;
	int maxID = 0; 

	for (auto i : stateIDs) {
		minID = std::min(minID, i.second);
		maxID = std::max(maxID, i.second);
	}

	std::vector<GameObject*>::const_iterator first;
	std::vector<GameObject*>::const_iterator last;
	world->GetObjectIterators(first, last);

	for (auto i = first; i != last; ++i) 
	{
		NetworkObject* o = (*i)->GetNetworkObject();
		if (!o) 
			continue;
		o->UpdateStateHistory(minID); 
	}*/
}

GameObject* NetworkedGame::GetPlayerPrefab(NetworkSpawnData* spawnPacket) 
{
	return TutorialGame::AddPlayerToWorld(Vector3(90, 22, -50), spawnPacket);
}


void NetworkedGame::SpawnPlayerClient(int ownerId, int objectId, Prefab prefab)
{
	// Will be prefab reference in the future
	NetworkSpawnData data = NetworkSpawnData();
	bool clientOwned = ownerId == thisClient->GetPeerId();

	data.clientOwned = clientOwned;
	data.objId = objectId;
	data.ownId = ownerId;
	auto object = GetPlayerPrefab(&data);

	if (clientOwned)
		ownedObjects.emplace_back(object);
}

void NetworkedGame::SpawnPlayerServer(int ownerId, Prefab prefab)
{
	// Will be prefab reference in the future

	NetworkSpawnData data = NetworkSpawnData();
	bool serverOwned = ownerId == thisServer->GetPeerId();
	data.clientOwned = serverOwned;
	data.objId = nextObjectId;
	data.ownId = ownerId;
	auto object = GetPlayerPrefab(&data);

	if (serverOwned)
		ownedObjects.emplace_back(object);

	SpawnPacket* newPacket = new SpawnPacket();
	newPacket->ownerId = ownerId;
	newPacket->objectId = nextObjectId;

	for (const auto& player : thisServer->playerPeers)
	{
		int playerID = player.first;
		thisServer->SendPacketToPeer(newPacket, playerID);
	}

	delete newPacket;
	nextObjectId++;
}

void NetworkedGame::SendSpawnPacketsOnClientConnect(int clientId)
{
	std::vector<GameObject*>::const_iterator first;
	std::vector<GameObject*>::const_iterator last;
	world->GetObjectIterators(first, last);

	for (auto i = first; i != last; ++i)
	{
		NetworkObject* o = (*i)->GetNetworkObject();

		if (!o)
			continue;

		SpawnPacket* newPacket = new SpawnPacket();
		newPacket->ownerId = o->GetOwnerID();
		newPacket->objectId = o->GetObjectID();
		thisServer->SendPacketToPeer(newPacket, clientId);

		delete newPacket;
	}
}



void NetworkedGame::StartLevel() 
{

}

void NetworkedGame::ReceivePacket(int type, GamePacket* payload, int source)
{
	if (type == Full_State || type == Delta_State) {

		if (thisServer)
			std::cout << "Recieved Client Transform packet" << std::endl;
		else 
			std::cout << "Recieved Server Transform packet" << std::endl;

		std::vector<GameObject*>::const_iterator first, last;
		world->GetObjectIterators(first, last);

		for (auto i = first; i != last; ++i)
		{
			NetworkObject* o = (*i)->GetNetworkObject();
			if (!o)
				continue;
			o->ReadPacket(*payload);
		}
	}
	if (type == Component_Event)
	{
		std::vector<INetworkComponent*>::const_iterator first, last;
		world->GetINetIterators(first, last);

		for (auto i = first; i != last; ++i)
		{
			INetworkComponent* c = (*i);
			INetworkPacket* p = (INetworkPacket*) payload;

			if (p->componentID != (c->GetComponentID()))
				continue;

			c->ReadEventPacket(*p);

			if (thisServer) 
			{
				for (const auto& player : thisServer->playerPeers)
				{
					int playerID = player.first;
					if (playerID != p->ownerID) {
						int playerID = player.first;
						thisServer->SendPacketToPeer(payload, playerID);
					}
				}
			}
		}
		
	}

	if (type == Spawn_Object) {
		if (thisClient) {
			SpawnPacket* ackPacket = (SpawnPacket*)payload;
			SpawnPlayerClient(ackPacket->ownerId, ackPacket->objectId, Prefab::Player);
		}
	}

	if (thisClient) 
		thisClient->ReceivePacket(type, payload, source);
	else if (thisServer) 
		thisServer->ReceivePacket(type, payload, source);
}

void NetworkedGame::OnPlayerCollision(NetworkPlayer* a, NetworkPlayer* b) {
	if (thisServer) 
	{ 
		MessagePacket newPacket;
		newPacket.messageID = COLLISION_MSG;
		newPacket.playerID  = a->GetPlayerNum();

		thisClient->SendPacket(newPacket);

		newPacket.playerID = b->GetPlayerNum();
		thisClient->SendPacket(newPacket);

	}
}