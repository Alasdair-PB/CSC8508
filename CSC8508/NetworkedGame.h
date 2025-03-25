#pragma once
#include "TutorialGame.h"
#include "NetworkBase.h"
#include "EventListener.h"

namespace NCL {
	namespace CSC8508 {
		class GameServer;
		class GameClient;
		class NetworkPlayer;
		enum Prefab { Player, EnemyA };

		class HostLobbyConnectEvent : public Event {};
		class ClientLobbyConnectEvent : public Event 
		{
			ClientLobbyConnectEvent(char a, char b, char c, char d) : a(a), b(b), c(c), d(d) {}
			ClientLobbyConnectEvent() : a(), b(), c(), d() {}
		public:
			char a;
			char b; 
			char c; 
			char d;
		};

		class NetworkedGame : 
			public TutorialGame,
			public PacketReceiver, 
			public EventListener<NetworkEvent>, 
			public EventListener<ClientConnectedEvent>,
			public EventListener<HostLobbyConnectEvent>,
			public EventListener<ClientLobbyConnectEvent> {
		public:
			NetworkedGame();
			~NetworkedGame();

			void StartAsServer();
			void StartAsClient(char a, char b, char c, char d);

			void UpdateGame(float dt) override;

			void SpawnPlayerClient(int ownerId, int objectId, Prefab prefab);
			void SpawnPlayerServer(int ownerId, Prefab prefab);

			void StartLevel();
			void ReceivePacket(int type, GamePacket* payload, int source) override;

			void OnEvent(ClientConnectedEvent* e) override;
			void OnEvent(NetworkEvent* e) override;
			void OnEvent(HostLobbyConnectEvent* e) override;
			void OnEvent(ClientLobbyConnectEvent* e) override;

		protected:
			void UpdatePackets(float dt);

			void StartClientCallBack();
			void StartServerCallBack();
			void StartOfflineCallBack();

			void SendSpawnPacketsOnClientConnect(int clientId);
			void BroadcastOwnedObjects(bool deltaFrame);

			bool SendToAllClients(GamePacket* dataPacket);

			void ReceiveComponentEventPacket(int type, GamePacket* payload);
			void ReceiveFullDeltaStatePacket(int type, GamePacket* payload);
			void ReceiveSpawnPacket(int type, GamePacket* payload);
			bool SendToAllOtherClients(GamePacket* dataPacket, int ownerId);

			GameServer* thisServer;
			GameClient* thisClient;

			float timeToNextPacket;
			int packetsToSnapshot;
			int nextObjectId;

			vector<GameObject*> ownedObjects;
			GameObject* GetPlayerPrefab(NetworkSpawnData* spawnPacket = nullptr);
			std::vector<int> playerStates;


		};
	}
}

