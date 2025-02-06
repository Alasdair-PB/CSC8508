#pragma once
#include "TutorialGame.h"
#include "NetworkBase.h"
#include "NetworkObject.h"

namespace NCL {
	namespace CSC8508 {
		class GameServer;
		class GameClient;
		class NetworkPlayer;

		class NetworkedGame : public TutorialGame, public PacketReceiver {
		public:
			NetworkedGame();
			~NetworkedGame();

			void StartAsServer();
			void StartAsClient(char a, char b, char c, char d);

			void UpdateGame(float dt) override;

			void SpawnPlayerClient(int ownerId, int objectId, GameObject* object);
			void SpawnPlayerServer(int ownerId, GameObject* object);


			void StartLevel();

			void ReceivePacket(int type, GamePacket* payload, int source) override;

			void OnPlayerCollision(NetworkPlayer* a, NetworkPlayer* b);

		protected:
			void UpdateAsServer(float dt);
			void UpdateAsClient(float dt);

			void StartClientCallBack();
			void StartServerCallBack();

			void StartOfflineCallBack();

			void SendSpawnPacketsOnClientConnect(int clientId);
			void BroadcastSnapshot(bool deltaFrame);
			void BroadcastOwnedObjects(bool deltaFrame);
			void SpawnNetworkedObject(int ownerId, int objectId, GameObject* object);
			void UpdateMinimumState();
			std::map<int, int> stateIDs;

			GameServer* thisServer;
			GameClient* thisClient;
			float timeToNextPacket;
			int packetsToSnapshot;

			int nextObjectId;

			std::map<int, GameObject*> ownedObjects;
			GameObject* GetPlayerPrefab();
			std::vector<int> playerStates;


		};
	}
}

