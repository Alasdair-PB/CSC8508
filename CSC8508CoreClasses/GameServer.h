#pragma once
#include "NetworkBase.h"
#include <unordered_map>

namespace NCL {
	namespace CSC8508 {
		class GameWorld;
		class GameServer : public NetworkBase {
		public:
			GameServer(int onPort, int maxClients);
			~GameServer();

			bool Initialise();
			void Shutdown();

			void SetGameWorld(GameWorld &g);

			bool SendGlobalPacket(int msgID);
			bool SendGlobalPacket(GamePacket& packet);
			bool SendPacketToPeer(GamePacket* packet, int playerID);
			void ReceivePacket(int type, GamePacket* payload, int source);


			std::unordered_map<int, _ENetPeer*> playerPeers;

			virtual void UpdateServer();

		protected:
			int			port;
			int			clientMax;
			int			clientCount;

			GameWorld*	gameWorld;
			std::unordered_map<int, int> playerStates;


			int incomingDataRate;
			int outgoingDataRate;
		};
	}
}
