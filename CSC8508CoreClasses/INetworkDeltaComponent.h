//
// Contributors: Alasdair
//

#ifndef INETWORKDELTACOMPONENT_H
#define INETWORKDELTACOMPONENT_H

#include "Transform.h"
#include "IComponent.h"
#include "NetworkBase.h"
#include "Event.h"
#include "EventManager.h"
#include "INetworkComponent.h"
using std::vector;

namespace NCL::CSC8508 
{	
	class INetworkState {
	public:
		INetworkState() : stateID(0) {}
		virtual ~INetworkState() = default;
		int stateID;
	};

	struct IDeltaNetworkPacket : INetworkPacket
	{
		int fullID = -1;

		IDeltaNetworkPacket() {
			type = Delta_State;
			size = sizeof(IDeltaNetworkPacket) - sizeof(GamePacket);
		}
	};

	struct IFullNetworkPacket : INetworkPacket
	{
		INetworkState fullState;

		IFullNetworkPacket() {
			type = Full_State;
			size = sizeof(IFullNetworkPacket) - sizeof(GamePacket);
		}
	};

	class INetworkDeltaComponent : public INetworkComponent
	{
	public:
		INetworkDeltaComponent(int objId, int ownId, int componentId, bool clientOwned, INetworkState* state = nullptr);
		virtual ~INetworkDeltaComponent() { delete lastFullState; }

		vector<GamePacket*> WriteDeltaFullPacket(bool deltaFrame);
		bool ReadDeltaFullPacket(INetworkPacket& p);

		void UpdateStateHistory(int minID);

	protected:
		bool GetNetworkState(int frameID, INetworkState* state);
		bool ReadDeltaPacketState(IDeltaNetworkPacket& p);
		virtual bool ReadDeltaPacket(IDeltaNetworkPacket& p) { return false; }
		virtual bool ReadFullPacket(IFullNetworkPacket& p) { return false; }

		virtual vector<GamePacket*> WriteDeltaPacket(bool* deltaFrame, int stateID) { return vector<GamePacket*>(); }
		virtual vector<GamePacket*> WriteFullPacket(){ return vector<GamePacket*>(); }

		INetworkState* lastFullState = nullptr;
		std::vector<INetworkState*> stateHistory;

		int deltaErrors;
		int fullErrors;
	};
}

#endif //INETWORKDELTACOMPONENT_H
