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
	auto dataPacket = e->eventData;
	if (thisServer) 
		SendToAllClients(dataPacket);
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

bool NetworkedGame::SendToAllClients(GamePacket* dataPacket)
{
	if (!thisServer)
		return false;

	for (const auto& player : thisServer->playerPeers)
	{
		int playerID = player.first;
		thisServer->SendPacketToPeer(dataPacket, playerID);
	}
	return true;
}


bool NetworkedGame::SendToAllOtherClients(GamePacket* dataPacket, int ownerId)
{
	if (!thisServer)
		return false;

	for (const auto& player : thisServer->playerPeers)
	{
		int playerID = player.first;
		if (playerID != ownerId)
			thisServer->SendPacketToPeer(dataPacket, playerID);
	}
	return true;
}

// Could be reworked if the client is able to learn their owning Id
// Client send all objects in their world not owned by server
void NetworkedGame::BroadcastOwnedObjects(bool deltaFrame) 
{
	ComponentManager::OperateOnINetworkComponents(
		[&](INetworkComponent* c) {
			if (thisServer)
			{
				GamePacket* newPacket = new GamePacket();
				if (c->IsOwner()) {
					/*if (*c->WriteDeltaPacket(&newPacket, deltaFrame, o->GetLatestNetworkState().stateID))
					{
						if (thisServer)
							SendToAllClients(*newPacket);
						else if (thisClient)
							thisClient->SendPacket(*newPacket);

						std::cout << "sending delata packet " << o->GetObjectID() << std::endl;

					}*/
				}
				delete newPacket;
			}
		});
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


GameObject* NetworkedGame::GetPlayerPrefab(NetworkSpawnData* spawnPacket) 
{
	return TutorialGame::AddPlayerToWorld(Vector3(90, 22, -50), spawnPacket);
}


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



void NetworkedGame::StartLevel() 
{

}

void NetworkedGame::ReceivePacket(int type, GamePacket* payload, int source)
{
	if (type == Full_State || type == Delta_State) {

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
	if (type == Component_Event) {
		INetworkPacket* p = (INetworkPacket*) payload;
		ComponentManager::OperateOnAllIcomponents(
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