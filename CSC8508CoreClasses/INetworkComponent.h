//
// Contributors: Alasdair
//

#ifndef INETWORKCOMPONENT_H
#define INETWORKCOMPONENT_H

#include "Transform.h"
#include "IComponent.h"
#include "NetworkBase.h"
#include "NetworkState.h"
#include "Event.h"

using std::vector;

namespace NCL::CSC8508 
{
	struct INetworkPacket : GamePacket 
	{
		int objectID = -1;
		int componentID = -1;
		short packetSubType = 0;
		int state = -1;

		virtual void GetData() {}

		INetworkPacket() {
			size = sizeof(INetworkPacket) - sizeof(GamePacket);
		}
	};

	class NetworkEvent : Event 
	{	
	public:		
		NetworkEvent(GamePacket* eventData) { this->eventData = eventData; }
		GamePacket* eventData;
	protected:
		GamePacket* GetEventData() const { return this->eventData; }
	};

	class INetworkComponent
	{
	public:
		INetworkComponent(int objId, int ownId, bool clientOwned);

		virtual ~INetworkComponent() = default;

		virtual bool ReadPacket(GamePacket& p) {}

		int GetObjectID() { return objectID; }
		int GetComponentID() { return objectID; }
		int GetOwnerID() { return ownerID; }

		vector<GamePacket*> WritePacket(bool deltaFrame, int stateID);
		void UpdateStateHistory(int minID);
		NetworkState& GetLatestNetworkState();

	protected:
		virtual bool ReadEventPacket(INetworkPacket& p) {}

		int objectID;
		int ownerID;
		bool clientOwned;
		int componentID;
		
		virtual bool ReadDeltaPacket(INetworkPacket& p) {}
		virtual bool ReadFullPacket(INetworkPacket& p) {}
		virtual vector<GamePacket*> WriteDeltaPacket(bool* deltaFrame, int stateID) {}
		virtual vector<GamePacket*> WriteFullPacket() {}
		bool GetNetworkState(int frameID, NetworkState& state);

		NetworkState lastFullState;
		std::vector<NetworkState> stateHistory;

		int deltaErrors;
		int fullErrors;
	};
}

#endif //ICOMPONENT_H
