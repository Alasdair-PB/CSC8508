#pragma once
#include "TutorialGame.h"
#include "NetworkBase.h"
#include "EventListener.h"

#if EOSBUILD

#include "EOSLobbyManager.h"
#include "EOSLobbySearch.h"
#include "EOSLobbyFunctions.h"

#endif

namespace NCL {
	namespace CSC8508 {
		class GameServer;
		class GameClient;
		class NetworkPlayer;
		enum Prefab { Player, Enemy, Item, Manager };

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

#if EOSBUILD
			std::string GetOwnerIP() const { return eosLobbyFunctions ? eosLobbyFunctions->ownerIP : ""; }
			std::string GetLobbyID() const { return eosLobbyFunctions ? eosLobbyFunctions->lobbyID : ""; }
			int GetPlayerCount() const { return eosLobbyFunctions ? eosLobbyFunctions->playerCount : 0; }
#endif

			void StartAsServer();
			void StartAsClient(char a, char b, char c, char d);

			void HostGame();

#if EOSBUILD
			void EOSLobbyCreation();
			void EOSLobbySearchFunc(const std::string& lobbyID);
			void EOSLobbyDetailsUpdate();
			void EOSStartAsHost();
			void EOSStartAsJoin(uint8_t a, uint8_t b, uint8_t c, uint8_t d);
#endif


			void UpdateGame(float dt) override;

			void SpawnObjectClient(int ownerId, int objectId, size_t pfab);
			void SpawnObjectServer(int ownerId, size_t prefab);

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

#if EOSBUILD
			void StartEOSCallBack();
			void StartAsHostCallBack();
			void StartAsJoinCallBack(const std::string& code);
			void StartEOSLobbyCreationCallBack();
			void StartEOSLobbySearchCallBack(const std::string& lobbyID);
			void StartEOSLobbyUpdateCallBack();

			EOSInitialisationManager* eosManager = new EOSInitialisationManager();
			EOSLobbyManager* eosLobbyManager = new EOSLobbyManager(*eosManager);
			EOSLobbySearch* eosLobbySearch = new EOSLobbySearch(*eosManager);
			EOSLobbyFunctions* eosLobbyFunctions = nullptr;
#endif

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

			std::vector<int> playerStates;
			vector<GameObject*> ownedObjects;
			GameObject* GetObjectFromPfab(size_t pfab, NetworkSpawnData data);
			GameObject* GetPlayerPrefab(NetworkSpawnData* spawnPacket = nullptr);
			GameObject* GetItemPrefab(NetworkSpawnData* spawnPacket = nullptr);
			GameObject* GetGameManagerPrefab(NetworkSpawnData* spawnPacket = nullptr);
		};
	}
}

