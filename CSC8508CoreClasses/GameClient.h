#pragma once
#include "NetworkBase.h"

namespace NCL {
	namespace CSC8508 {
		class GameClient : public NetworkBase {
		public:
			GameClient();
			~GameClient();

			bool Connect(uint8_t a, uint8_t b, uint8_t c, uint8_t d, int portNum);

			void SendPacket(GamePacket&  payload);
			void ReceivePacket(int type, GamePacket* payload, int source);
			void UpdateClient();
		protected:	
			_ENetPeer*	netPeer;
			int lastAcknowledgedStateID;
		};
	}
}

