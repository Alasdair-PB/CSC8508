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
#include "EventManager.h"

using std::vector;

namespace NCL::CSC8508 
{
	struct INetworkPacket : GamePacket 
	{
		int ownerID = -1;
		int componentID = -1;
		short packetSubType = 0;

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
		INetworkComponent(int objId, int ownId, int componentID, bool clientOwned);

		virtual ~INetworkComponent() = default;
		virtual bool ReadEventPacket(INetworkPacket& p) { return false; }

		int GetObjectID() { return objectID; }
		int GetComponentID() { return componentID; }
		int GetOwnerID() { return ownerID; }

		/*vector<GamePacket*> WritePacket(bool deltaFrame, int stateID);
		void UpdateStateHistory(int minID);
		NetworkState& GetLatestNetworkState();*/

	protected:

		int objectID;
		int ownerID;
		bool clientOwned;
		int componentID;

		void SendEventPacket(INetworkPacket* packet)
		{
			packet->componentID = this->componentID;
			packet->ownerID = this->ownerID;

			NetworkEvent networkPacket = NetworkEvent(packet);
			EventManager::Call<NetworkEvent>(&networkPacket);
		}

		/*virtual bool ReadDeltaPacket(INetworkPacket& p) {}
		virtual bool ReadFullPacket(INetworkPacket& p) {}
		virtual vector<GamePacket*> WriteDeltaPacket(bool* deltaFrame, int stateID) {}
		virtual vector<GamePacket*> WriteFullPacket() {}
		bool GetNetworkState(int frameID, NetworkState& state);

		NetworkState lastFullState;
		std::vector<NetworkState> stateHistory;

		int deltaErrors;
		int fullErrors;*/
	};
}

#endif //ICOMPONENT_H
