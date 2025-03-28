//
// Original author: Rich Davison
// Contributors: Alasdair
//

#include "NetworkedGame.h"
#include "GameServer.h"
#include "RenderObject.h"
#include "INetworkComponent.h"
#include "INetworkDeltaComponent.h"
#include "ComponentManager.h"
#include "EventManager.h"
#include "GameClient.h"
#include "GameManagerComponent.h"

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
	short pfabId;

	SpawnPacket() {
		type = Spawn_Object;
		size = sizeof(SpawnPacket) - sizeof(GamePacket);
	}
};

void NetworkedGame::StartClientCallBack() { StartAsClient(10, 70, 33, 111); } //IP config
void NetworkedGame::StartServerCallBack() { StartAsServer(); }
void NetworkedGame::StartOfflineCallBack() { 
	TutorialGame::LoadGameManager(Vector3(93, 22, -53));
	TutorialGame::AddPlayerToWorld(Vector3(90, 22, -50)); 
	TutorialGame::Loaditem(Vector3(93, 22, -53));
}

#if EOSBUILD

void NetworkedGame::StartEOSCallBack() { HostGame(); }
void NetworkedGame::StartEOSLobbyCreationCallBack() { EOSLobbyCreation(); }
void NetworkedGame::StartEOSLobbySearchCallBack(const std::string& lobbyID) {EOSLobbySearchFunc(lobbyID);}
void NetworkedGame::StartAsHostCallBack() {EOSStartAsHost();}

void NetworkedGame::StartAsJoinCallBack(const std::string& ip) {
	std::stringstream ss(ip);
	std::string segment;
	std::vector<uint8_t> bytes;

	while (std::getline(ss, segment, '.')) {
		int num = std::stoi(segment);
		if (num < 0 || num > 255) {
			std::cout << "Invalid IP segment: " << segment << std::endl;
			return;
		}
		bytes.push_back(static_cast<uint8_t>(num));
	}

	if (bytes.size() != 4) {
		std::cout << "Invalid IP format: " << ip << std::endl;
		return;
	}

	std::cout << "Parsed IP: " << ip << std::endl;

	uint8_t a = bytes[0];
	uint8_t b = bytes[1];
	uint8_t c = bytes[2];
	uint8_t d = bytes[3];

	EOSStartAsJoin(a, b, c, d);
}

void NetworkedGame::StartEOSLobbyUpdateCallBack() { EOSLobbyDetailsUpdate(); }

#endif

void NetworkedGame::OnEvent(HostLobbyConnectEvent* e) { StartAsServer(); }
void NetworkedGame::OnEvent(ClientLobbyConnectEvent* e) { StartAsClient(e->a, e->b, e->c, e->d); }

NetworkedGame::NetworkedGame()	{
	EventManager::RegisterListener<NetworkEvent>(this);
	EventManager::RegisterListener<ClientConnectedEvent>(this);

	thisServer = nullptr;
	thisClient = nullptr;

#if !EOSBUILD
	mainMenu = new MainMenu(
		[&](bool state) -> void { world->SetPausedWorld(state); },
		[&]() -> void { this->StartClientCallBack(); },
		[&]() -> void { this->StartServerCallBack(); },
		[&]() -> void { this->StartOfflineCallBack(); }
	);
#else
	mainMenu = new MainMenu(
		[&](bool state) -> void { world->SetPausedWorld(state); },
		[&]() -> void { this->StartClientCallBack(); },
		[&]() -> void { this->StartServerCallBack(); },
		[&]() -> void { this->StartOfflineCallBack(); },
		[&]() -> void { this->StartEOSCallBack(); },
		[&]() -> void { this->StartEOSLobbyCreationCallBack(); },
		[&](std::string id) -> void { this->StartEOSLobbySearchCallBack(id); },
		[&]() -> void { this->StartEOSLobbyUpdateCallBack(); },
		[&]() -> std::string { return this->GetOwnerIP(); },
		[&]() -> std::string { return this->GetLobbyID(); },
		[&]() -> int { return this->GetPlayerCount(); },
		[&]() -> void { this->StartAsHostCallBack(); },
		[&](const std::string& code) -> void { this->StartAsJoinCallBack(code); }
	);
#endif

	NetworkBase::Initialise();
	timeToNextPacket  = 0.0f;
	packetsToSnapshot = 0;
	playerStates = std::vector<int>();

}

NetworkedGame::~NetworkedGame()	{
	if(thisServer) delete thisServer;
	if (thisClient) delete thisClient;
	TutorialGame::~TutorialGame();
}

void NetworkedGame::StartAsServer() 
{
	thisServer = new GameServer(NetworkBase::GetDefaultPort(), 4);
	thisServer->RegisterPacketHandler(Received_State, this);
	thisServer->RegisterPacketHandler(Spawn_Object, this);
	thisServer->RegisterPacketHandler(Component_Event, this);

	thisServer->RegisterPacketHandler(Delta_State, this);
	thisServer->RegisterPacketHandler(Full_State, this);
	
	SpawnObjectServer(thisServer->GetPeerId(), Prefab::Manager);
	SpawnObjectServer(thisServer->GetPeerId(), Prefab::Player);
	SpawnObjectServer(thisServer->GetPeerId(), Prefab::Item);
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

#if EOSBUILD
void NetworkedGame::HostGame()
{

	eosManager->StartEOS();
}

void NetworkedGame::EOSLobbyCreation()
{
	eosLobbyManager->CreateLobby();
	eosLobbySearch->CreateLobbySearch(eosLobbyManager->LobbyId);

	eosLobbyFunctions = new EOSLobbyFunctions(*eosManager, *eosLobbySearch);
}

void NetworkedGame::EOSLobbySearchFunc(const std::string& lobbyID)
{

	eosLobbySearch->CreateLobbySearch(lobbyID.c_str()); // <-- convert std::string to const char*

	eosLobbyFunctions = new EOSLobbyFunctions(*eosManager, *eosLobbySearch);
	eosLobbyFunctions->JoinLobby();

}

void NetworkedGame::EOSLobbyDetailsUpdate()
{

	eosLobbyFunctions->UpdateLobbyDetails();

}

void NetworkedGame::EOSStartAsHost()
{

	thisServer = new GameServer(NetworkBase::GetDefaultPort(), 4);
	thisServer->RegisterPacketHandler(Received_State, this);
	thisServer->RegisterPacketHandler(Spawn_Object, this);
	thisServer->RegisterPacketHandler(Component_Event, this);

	thisServer->RegisterPacketHandler(Delta_State, this);
	thisServer->RegisterPacketHandler(Full_State, this);
	SpawnObjectServer(thisServer->GetPeerId(), Prefab::Player);

}

void NetworkedGame::EOSStartAsJoin(uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
	std::cout << static_cast<int>(a) << std::endl;
	std::cout << static_cast<int>(b) << std::endl;
	std::cout << static_cast<int>(c) << std::endl;
	std::cout << static_cast<int>(d) << std::endl;

	thisClient = new GameClient();
	thisClient->Connect(a, b, c, d, NetworkBase::GetDefaultPort());

	thisClient->RegisterPacketHandler(Delta_State, this);
	thisClient->RegisterPacketHandler(Full_State, this);
	thisClient->RegisterPacketHandler(Component_Event, this);
	thisClient->RegisterPacketHandler(Player_Connected, this);
	thisClient->RegisterPacketHandler(Player_Disconnected, this);
	thisClient->RegisterPacketHandler(Spawn_Object, this);
}

#endif
void NetworkedGame::OnEvent(ClientConnectedEvent* e) 
{
	int id = e->GetClientId();
	SendSpawnPacketsOnClientConnect(id);	
	SpawnObjectServer(id, Prefab::Player);
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

// To change to Pfab managed system in the future where all pfabs 
// are loaded as disabled GameObjects that can be copied to new objects during runtime
GameObject* NetworkedGame::GetPlayerPrefab(NetworkSpawnData* spawnPacket) 
	{ return TutorialGame::AddPlayerToWorld(Vector3(90, 22, -50), spawnPacket);}

GameObject* NetworkedGame::GetItemPrefab(NetworkSpawnData* spawnPacket)
	{ return TutorialGame::Loaditem(Vector3(93, 22, -53), spawnPacket);}

GameObject* NetworkedGame::GetGameManagerPrefab(NetworkSpawnData* spawnPacket)
	{ return TutorialGame::LoadGameManager(Vector3(93, 22, -53), spawnPacket);}

GameObject* NetworkedGame::GetObjectFromPfab(size_t pfab, NetworkSpawnData data) {
	GameObject* object = nullptr;
	if (pfab == Prefab::Player)
		object = GetPlayerPrefab(&data);
	else if (pfab == Prefab::Manager)
		object = GetGameManagerPrefab(&data);
	else if (pfab == Prefab::Item)
		object = GetItemPrefab(&data);
	return object;
}

void NetworkedGame::SpawnObjectClient(int ownerId, int objectId, size_t pfab)
{
	bool clientOwned = ownerId == thisClient->GetPeerId();
	NetworkSpawnData data = NetworkSpawnData();

	data.clientOwned = clientOwned;
	data.objId = objectId;
	data.ownId = ownerId;
	data.pfab = pfab;
	GameObject* object = GetObjectFromPfab(pfab, data);

	if (clientOwned)
		ownedObjects.emplace_back(object);
}

void NetworkedGame::SpawnObjectServer(int ownerId, size_t pfab)
{
	bool serverOwned = ownerId == thisServer->GetPeerId();
	NetworkSpawnData data = NetworkSpawnData();

	data.clientOwned = serverOwned;
	data.objId = nextObjectId;
	data.ownId = ownerId;
	data.pfab = pfab;

	GameObject* object = GetObjectFromPfab(pfab, data);

	if (serverOwned)
		ownedObjects.emplace_back(object);

	SpawnPacket* newPacket = new SpawnPacket();
	newPacket->ownerId = ownerId;
	newPacket->objectId = nextObjectId;
	newPacket->pfabId = pfab;

	SendToAllClients(newPacket);

	delete newPacket;
	nextObjectId++;
}

// Change to addNetworkObject component in the future so this info does not need to be included on every GameObject
bool HasNetworkComponent(GameObject* object, int& objectId, int& ownerId, int& pFabId) {
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
			pFabId = networkComponent->GetPfabID();
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
	int pfabId;

	for (auto i = first; i != last; ++i)
	{
		if (HasNetworkComponent(*i, objectId, ownerId, pfabId))
		{
			SpawnPacket* newPacket = new SpawnPacket();
			newPacket->ownerId = ownerId;
			newPacket->objectId = objectId;
			newPacket->pfabId = pfabId;
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
			SpawnObjectClient(ackPacket->ownerId, ackPacket->objectId, ackPacket->pfabId);
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