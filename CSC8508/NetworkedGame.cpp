//
// Original author: Rich Davison
// Contributors: Alasdair
//

#include "NetworkedGame.h"
#include "NetworkPlayer.h"
#include "NetworkObject.h"
#include "GameServer.h"
#include "GameClient.h"
#include "RenderObject.h"
#include "INetworkComponent.h"
#include "INetworkDeltaComponent.h"
#include "ComponentManager.h"
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
		size = sizeof(SpawnPacket) - sizeof(GamePacket);
	}
};

void NetworkedGame::StartClientCallBack() { StartAsClient(10, 70, 33, 111); } //IP config
void NetworkedGame::StartServerCallBack() { StartAsServer(); }
void NetworkedGame::StartOfflineCallBack() { TutorialGame::AddPlayerToWorld(Vector3(90, 22, -50)); }


void NetworkedGame::OnEvent(HostLobbyConnectEvent* e) { StartAsServer(); }
void NetworkedGame::OnEvent(ClientLobbyConnectEvent* e) { StartAsClient(e->a, e->b, e->c, e->d); }

NetworkedGame::NetworkedGame()	{
	EventManager::RegisterListener<NetworkEvent>(this);
	EventManager::RegisterListener<ClientConnectedEvent>(this);

	thisServer = nullptr;
	thisClient = nullptr;

	mainMenu = new MainMenu([&](bool state) -> void {world->SetPausedWorld(state); },
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
	auto dataPacket = e->eventData;
	if (thisServer) 
		SendToAllClients(dataPacket);
	else 
		thisClient->SendPacket(*dataPacket);
}

//20hz server/client update
void NetworkedGame::UpdateGame(float dt) 
{
	timeToNextPacket -= dt;
	if (timeToNextPacket < 0) {
		UpdatePackets(dt);
		timeToNextPacket += 1.0f; // 1.0f / 20.0f;
	}

	if (thisServer) 
		thisServer->UpdateServer();
	else if (thisClient) 
		thisClient->UpdateClient();
	TutorialGame::UpdateGame(dt);
}

void NetworkedGame::UpdatePackets(float dt)
{
	packetsToSnapshot--;
	if (packetsToSnapshot < 0) 
	{
		BroadcastOwnedObjects(false);
		packetsToSnapshot = 5;
	}
	else 
		BroadcastOwnedObjects(true);
}

bool NetworkedGame::SendToAllClients(GamePacket* dataPacket)
{
	if (!thisServer) return false;
	for (const auto& player : thisServer->playerPeers)
	{
		int playerID = player.first;
		thisServer->SendPacketToPeer(dataPacket, playerID);
	}
	return true;
}

bool NetworkedGame::SendToAllOtherClients(GamePacket* dataPacket, int ownerId)
{
	if (!thisServer) return false;
	for (const auto& player : thisServer->playerPeers)
	{
		int playerID = player.first;
		if (playerID != ownerId)
			thisServer->SendPacketToPeer(dataPacket, playerID);
	}
	return true;
}

void NetworkedGame::BroadcastOwnedObjects(bool deltaFrame) 
{
	ComponentManager::OperateOnAllINetworkDeltaComponentBufferOperators(
		[&](IComponent* ic) 
		{
			INetworkDeltaComponent* c = dynamic_cast<INetworkDeltaComponent*>(ic);
			if (!c) return;
			if (c->IsOwner()) {
				vector<GamePacket*> packets = c->WriteDeltaFullPacket(deltaFrame);
				for (GamePacket* packet : packets)
				{
					if (thisClient)
						thisClient->SendPacket(*packet);
					else if (thisServer)
						SendToAllOtherClients(packet, c->GetOwnerID());
					delete packet;
				}
				packets.clear();
			}
		});
}

GameObject* NetworkedGame::GetPlayerPrefab(NetworkSpawnData* spawnPacket) 
	{ return TutorialGame::AddPlayerToWorld(Vector3(90, 22, -50), spawnPacket);}

void NetworkedGame::SpawnPlayerClient(int ownerId, int objectId, Prefab prefab)
{
	// Will be prefab reference in the future	
	bool clientOwned = ownerId == thisClient->GetPeerId();
	NetworkSpawnData data = NetworkSpawnData();

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
	bool serverOwned = ownerId == thisServer->GetPeerId();
	NetworkSpawnData data = NetworkSpawnData();

	data.clientOwned = serverOwned;
	data.objId = nextObjectId;
	data.ownId = ownerId;
	auto object = GetPlayerPrefab(&data);

	if (serverOwned)
		ownedObjects.emplace_back(object);

	SpawnPacket* newPacket = new SpawnPacket();
	newPacket->ownerId = ownerId;
	newPacket->objectId = nextObjectId;

	SendToAllClients(newPacket);

	delete newPacket;
	nextObjectId++;
}

bool HasNetworkComponent(GameObject* object, int& objectId, int& ownerId) {
	if (!object)
		return false;

	for (auto component : object->GetAllComponents()) {
		if (component->IsDerived(typeid(INetworkComponent))) 
		{
			INetworkComponent* networkComponent = dynamic_cast<INetworkComponent*>(component);
			if (!networkComponent)
				std::cout << "NetworkComponent is not derived type" << std::endl;
			objectId = networkComponent->GetObjectID();
			ownerId = networkComponent->GetOwnerID();
			return true;
		}
	}
	return false;
}

void NetworkedGame::SendSpawnPacketsOnClientConnect(int clientId)
{
	std::vector<GameObject*>::const_iterator first;
	std::vector<GameObject*>::const_iterator last;
	world->GetObjectIterators(first, last);

	int ownerId;
	int objectId;
	for (auto i = first; i != last; ++i)
	{
		if (HasNetworkComponent(*i, objectId, ownerId))
		{
			SpawnPacket* newPacket = new SpawnPacket();
			newPacket->ownerId = ownerId;
			newPacket->objectId = objectId;
			thisServer->SendPacketToPeer(newPacket, clientId);
			delete newPacket;
		}
	}
}

void NetworkedGame::StartLevel() {}

void NetworkedGame::ReceiveFullDeltaStatePacket(int type, GamePacket* payload) {
	if (type == Full_State || type == Delta_State) {
		INetworkPacket* p = (INetworkPacket*)payload;
		ComponentManager::OperateOnAllINetworkDeltaComponentBufferOperators(
			[&](IComponent* ic) {
				INetworkDeltaComponent* c = dynamic_cast<INetworkDeltaComponent*>(ic);
				if (p->componentID == (c->GetComponentID()))
				{
					c->ReadDeltaFullPacket(*p);
					if (thisServer)
						SendToAllOtherClients(payload, p->ownerID);
				}
			});
	}
}

void NetworkedGame::ReceiveComponentEventPacket(int type, GamePacket* payload) {
	if (type == Component_Event) {
		INetworkPacket* p = (INetworkPacket*)payload;
		ComponentManager::OperateOnAllINetworkComponentBufferOperators(
			[&](IComponent* ic) {
				INetworkComponent* c = dynamic_cast<INetworkComponent*>(ic);
				if (p->componentID == (c->GetComponentID()))
				{
					c->ReadEventPacket(*p);
					if (thisServer)
						SendToAllOtherClients(payload, p->ownerID);
				}
			});
	}
}

void NetworkedGame::ReceiveSpawnPacket(int type, GamePacket* payload) {
	if (type == Spawn_Object) {
		if (thisClient) {
			SpawnPacket* ackPacket = (SpawnPacket*)payload;
			SpawnPlayerClient(ackPacket->ownerId, ackPacket->objectId, Prefab::Player);
		}
	}
}

void NetworkedGame::ReceivePacket(int type, GamePacket* payload, int source)
{
	ReceiveFullDeltaStatePacket(type, payload);
	ReceiveComponentEventPacket(type, payload);
	ReceiveSpawnPacket(type, payload);
	if (thisClient) thisClient->ReceivePacket(type, payload, source);
	else if (thisServer) thisServer->ReceivePacket(type, payload, source);
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