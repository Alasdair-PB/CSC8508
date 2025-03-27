//
// Contributors: Alasdair
//

#ifndef INETWORKCOMPONENT_H
#define INETWORKCOMPONENT_H

#include "Transform.h"
#include "IComponent.h"
#include "NetworkBase.h"
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
			type = Component_Event;
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
		INetworkComponent(int objId, int ownId, int componentID, int pfabId, bool clientOwned);

		virtual ~INetworkComponent() = default;
		virtual bool ReadEventPacket(INetworkPacket& p) { return false; }

		int GetObjectID() { return objectID; }
		int GetComponentID() { return componentID; }
		int GetOwnerID() { return ownerID; }
		int GetPfabID() { return pfabID;  }
		bool IsOwner() { return clientOwned; }

	protected:

		int objectID;
		int ownerID;
		bool clientOwned;
		int componentID;
		int pfabID;

		void SendEventPacket(INetworkPacket* packet)
		{
			packet->componentID = this->componentID;
			packet->ownerID = this->ownerID;

			NetworkEvent networkPacket = NetworkEvent(packet);
			EventManager::Call<NetworkEvent>(&networkPacket);
		}

		void SetPacketOwnership(INetworkPacket* packet) {
			packet->componentID = this->componentID;
			packet->ownerID = this->ownerID;
		}
	};
}

#endif //ICOMPONENT_H
