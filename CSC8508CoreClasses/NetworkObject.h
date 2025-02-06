#pragma once
#include "GameObject.h"
#include "NetworkBase.h"
#include "NetworkState.h"

namespace NCL::CSC8508 {
	class GameObject;

	struct FullPacket : public GamePacket {
		int		objectID = -1;
		NetworkState fullState;

		FullPacket() {
			type = Full_State;
			size = sizeof(FullPacket) - sizeof(GamePacket);
		}
	};

	struct DeltaPacket : public GamePacket {
		int		fullID		= -1;
		int		objectID	= -1;
		char	pos[3];
		char	orientation[4];

		DeltaPacket() {
			type = Delta_State;
			size = sizeof(DeltaPacket) - sizeof(GamePacket);
		}
	};

	struct SetClientId : public GamePacket {
		int clientPeerId;

		SetClientId() {
			size = sizeof(SetClientId) - sizeof(GamePacket);
			type = Player_Connected;
		}
	};


	class NetworkObject		
	{
	public:
		NetworkObject(GameObject& o, int objId, int ownId);
		virtual ~NetworkObject();
		virtual bool ReadPacket(GamePacket& p);
		virtual bool WritePacket(GamePacket** p, bool deltaFrame, int stateID);
		int GetObjectID() { return objectID; }
		int GetOwnerID() { return ownerID;  }
		void UpdateStateHistory(int minID);
		NetworkState& GetLatestNetworkState();

	protected:


		bool GetNetworkState(int frameID, NetworkState& state);

		virtual bool ReadDeltaPacket(DeltaPacket &p);
		virtual bool ReadFullPacket(FullPacket &p);

		virtual bool WriteDeltaPacket(GamePacket**p, int stateID);
		virtual bool WriteFullPacket(GamePacket**p);

		GameObject& object;
		NetworkState lastFullState;
		std::vector<NetworkState> stateHistory;

		int deltaErrors;
		int fullErrors;
		int objectID;
		int ownerID;
	};
}