#include "NetworkedGame.h"
#include "NetworkPlayer.h"
#include "NetworkObject.h"
#include "GameServer.h"
#include "GameClient.h"
#include "RenderObject.h"


#define COLLISION_MSG 30

struct MessagePacket : public GamePacket {
	short playerID;
	short messageID;

	MessagePacket() {
		type = Message;
		size = sizeof(short) * 2;
	}
};

void NetworkedGame::StartClientCallBack() { StartAsClient(127, 0, 0, 1); }
void NetworkedGame::StartServerCallBack() { StartAsServer(); }
void NetworkedGame::StartOfflineCallBack() { SpawnPlayer(); }


NetworkedGame::NetworkedGame()	{
	thisServer = nullptr;
	thisClient = nullptr;

	mainMenu = new MainMenu([&](bool state) -> void { this->SetPause(state); },
		[&]() -> void { this->StartClientCallBack(); },
		[&]() -> void { this->StartServerCallBack(); },
		[&]() -> void { this->StartOfflineCallBack(); });

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
	StartLevel();
}

void NetworkedGame::StartAsClient(char a, char b, char c, char d) 
{
	thisClient = new GameClient();
	thisClient->Connect(a, b, c, d, NetworkBase::GetDefaultPort());

	thisClient->RegisterPacketHandler(Delta_State, this);
	thisClient->RegisterPacketHandler(Full_State, this);

	thisClient->RegisterPacketHandler(Player_Connected, this);
	thisClient->RegisterPacketHandler(Player_Disconnected, this);

	StartLevel();
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
	else {
		BroadcastSnapshot(true);
	}

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
	else {
		ClientPacket newPacket;
		newPacket.score = score;
		newPacket.lastID = 0;
		thisClient->SendPacket(newPacket);
		BroadcastOwnedObjects(true);
	}
	thisClient->UpdateClient();
}


void NetworkedGame::BroadcastOwnedObjects(bool deltaFrame) {

	std::vector<GameObject*>::const_iterator first, last;
	world->GetObjectIterators(first, last);

	for (auto i = first; i != last; ++i)
	{
		NetworkObject* o = (*i)->GetNetworkObject();

		if (!o)
			continue;

		if (o->GetNetworkID() == 0) // id of server for now hard coded
			continue;

		GamePacket* newPacket = new GamePacket();
		newPacket->type = Full_State;

		if (o->WritePacket(&newPacket, deltaFrame, 0)) //lastAcknowledgedState
			thisClient->SendPacket(*newPacket);
		delete newPacket;
	}
}


void NetworkedGame::BroadcastSnapshot(bool deltaFrame) 
{
	for (const auto& player : thisServer->playerPeers)
	{	
		int playerID = player.first;
		//int lastAcknowledgedState = playerStates[playerID];
		std::vector<GameObject*>::const_iterator first, last;
		world->GetObjectIterators(first, last);

		for (auto i = first; i != last; ++i) 
		{
			NetworkObject* o = (*i)->GetNetworkObject();

			if (!o) 
				continue;

			GamePacket* newPacket = new GamePacket();
			newPacket->type = Full_State;

			if (o->WritePacket(&newPacket, deltaFrame, 0)) //lastAcknowledgedState
			{
				thisServer->SendPacketToPeer(newPacket, playerID);
				std::cout << "sending packet to peer" << std::endl;
			}				
			delete newPacket;
		}

		ClientPacket* scorePacket = new ClientPacket();
		scorePacket->score = score;
		scorePacket->lastID = 0;
		thisServer->SendPacketToPeer(scorePacket, playerID);
		delete scorePacket;
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

void NetworkedGame::SpawnPlayer() 
{
	auto play = TutorialGame::AddPlayerToWorld(Vector3(90, 22, -50));
	NetworkObject* player = new NetworkObject(*play, 0); 
	play->SetNetworkObject(player);
	world->AddGameObject(play);
}

void NetworkedGame::StartLevel() 
{
	//for (const auto& player : serverPlayers)
	//{
		SpawnPlayer();
	//}
}

void NetworkedGame::ReceivePacket(int type, GamePacket* payload, int source)
{
	std::vector<GameObject*>::const_iterator first, last;
	world->GetObjectIterators(first, last);
	for (auto i = first; i != last; ++i)
	{
		NetworkObject* o = (*i)->GetNetworkObject();
		if (!o)
			continue;
		o->ReadPacket(*payload);
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