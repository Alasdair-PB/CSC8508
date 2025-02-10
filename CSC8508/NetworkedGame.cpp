#include "NetworkedGame.h"
#include "NetworkPlayer.h"
#include "NetworkObject.h"
#include "GameServer.h"
#include "GameClient.h"
#include "RenderObject.h"
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
	EventManager::RegisterListener(this);
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

	thisServer->RegisterPacketHandler(Delta_State, this);
	thisServer->RegisterPacketHandler(Full_State, this);

	SpawnPlayerServer(thisServer->GetPeerId(), GetPlayerPrefab());
}

void NetworkedGame::StartAsClient(char a, char b, char c, char d) 
{
	thisClient = new GameClient();
	thisClient->Connect(a, b, c, d, NetworkBase::GetDefaultPort());

	thisClient->RegisterPacketHandler(Delta_State, this);
	thisClient->RegisterPacketHandler(Full_State, this);

	thisClient->RegisterPacketHandler(Player_Connected, this);
	thisClient->RegisterPacketHandler(Player_Disconnected, this);

	thisClient->RegisterPacketHandler(Spawn_Object, this);
}

void NetworkedGame::OnEvent(ClientConnectedEvent* e) 
{
	int id = e->GetClientId();
	SendSpawnPacketsOnClientConnect(id);	
	SpawnPlayerServer(id, GetPlayerPrefab());
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
	for (GameObject* i : ownedObjects) {
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
	}


}

// Server goes through each object in their world and sends delta or full packets for each. 
void NetworkedGame::BroadcastSnapshot(bool deltaFrame) 
{
	for (const auto& player : thisServer->playerPeers)
	{	
		int playerID = player.first;
		std::vector<GameObject*>::const_iterator first, last;
		world->GetObjectIterators(first, last);

		for (auto i = first; i != last; ++i) 
		{
			NetworkObject* o = (*i)->GetNetworkObject();
			if (!o) 
				continue;

			if (o->GetOwnerID() == playerID)
				continue;

			GamePacket* newPacket = new GamePacket();
			if (o->WritePacket(&newPacket, deltaFrame, o->GetLatestNetworkState().stateID))
			{
				thisServer->SendPacketToPeer(newPacket, playerID);
				std::cout << "sending packet to peer: " << playerID<< ", " << o->GetObjectID() << std::endl;
			}				
			delete newPacket;
		}
	}
}

void NetworkedGame::UpdateMinimumState() 
{
	int minID = INT_MAX;
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
	}
}

GameObject* NetworkedGame::GetPlayerPrefab() {
	return TutorialGame::AddPlayerToWorld(Vector3(90, 22, -50));
}

void NetworkedGame::SpawnNetworkedObject(int ownerId, int objectId, GameObject* object)
{
	NetworkObject* netObject = new NetworkObject(*object, objectId, ownerId);
	object->SetNetworkObject(netObject);
}

void NetworkedGame::SpawnPlayerClient(int ownerId, int objectId, GameObject* object)
{
	SpawnNetworkedObject(ownerId, objectId, object);
	if (thisClient->GetPeerId() == ownerId)
		ownedObjects.emplace_back(object);
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

void NetworkedGame::SpawnPlayerServer(int ownerId, GameObject* object)
{
	SpawnNetworkedObject(ownerId, nextObjectId, object);

	if (thisServer->GetPeerId() == ownerId)
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

	if (type == Spawn_Object) {
		if (thisClient) {
			SpawnPacket* ackPacket = (SpawnPacket*)payload;
			SpawnPlayerClient(ackPacket->ownerId, ackPacket->objectId, GetPlayerPrefab());
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