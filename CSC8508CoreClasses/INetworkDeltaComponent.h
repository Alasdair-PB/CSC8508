//
// Contributors: Alasdair
//

#ifndef INETWORKDELTACOMPONENT_H
#define INETWORKDELTACOMPONENT_H

#include "IComponent.h"
#include "NetworkBase.h"
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
		INetworkDeltaComponent(int objId, int ownId, int componentId, int pfabID, bool clientOwned, INetworkState* state = nullptr);
		virtual ~INetworkDeltaComponent() { delete lastFullState; }

		vector<GamePacket*> WriteDeltaFullPacket(bool deltaFrame);
		bool ReadDeltaFullPacket(INetworkPacket& p);
		void UpdateStateHistory(int minID);

	protected:	
		const int MAX_PACKETID = 1000;
		const int CLEAN_HISTORY_FRAME_DELAY = 100;

		bool ReadDeltaPacketState(IDeltaNetworkPacket& p);
		bool ReadFullPacketState(IFullNetworkPacket& p);
		virtual bool ReadDeltaPacket(IDeltaNetworkPacket& p) { return false; }
		virtual bool ReadFullPacket(IFullNetworkPacket& p) { return false; }

		virtual vector<GamePacket*> WriteDeltaPacket(bool* deltaFrame) { return vector<GamePacket*>(); }
		virtual vector<GamePacket*> WriteFullPacket(){ return vector<GamePacket*>(); }

		INetworkState* lastFullState = nullptr;
		std::vector<INetworkState*> stateHistory;

		int deltaErrors;
		int fullErrors;

		/// <summary>
		/// Gets a state of type T from the stateHistory
		/// </summary>
		/// <typeparam name="T">The derived state type</typeparam>
		/// <param name="state">A pointer to the returned state</param>
		/// <returns>True if stateHistory found, otherwise returns false</returns>
		template<typename T>
		bool GetNetworkState(T* state) {
			if (state == nullptr)
				return false;

			for (int i = 0; i < stateHistory.size(); i++) {
				if (state->stateID == stateHistory[i]->stateID) {
					T* historyState = (T*)stateHistory[i];
					state = historyState;
					return true;
				}
			}
			return false;
		}

		/// <summary>
		/// Updates history stateId to the most recent state Id. The logic has been expanded to reset the id when the id becomes greater 
		/// than MAX_PACKETID to avoid an overload. 
		/// </summary>
		/// <typeparam name="T">The derived state type</typeparam>
		/// <typeparam name="T2">The derived packet type</typeparam>
		/// <param name="p">The packet recieved</param>
		/// <param name="newStateId">The next stateId to be pushed to history</param>
		/// <returns>True if the packet is the most recent packet otherwise returns false</returns>
		template <typename T, typename T2>
		bool UpdateFullStateHistory(T2 p, int* newStateId) {
			int lastState = ((T*)lastFullState)->stateID;
			int packetFullState = p.fullState.stateID;
			int modifiedPFullState = p.fullState.stateID - MAX_PACKETID;
			int modifiedLastState = lastState - MAX_PACKETID;

			if (packetFullState < MAX_PACKETID) {
				if (lastState != 0 && (packetFullState < lastState ||
					((lastState < (MAX_PACKETID / 2)) && (packetFullState > (MAX_PACKETID / 2)))))
					return false;
				else {
					*newStateId = packetFullState;
					if (*newStateId % CLEAN_HISTORY_FRAME_DELAY == 0) UpdateStateHistory(*newStateId);
				}
			}
			else if (packetFullState >= MAX_PACKETID) {
				if (lastState > (MAX_PACKETID / 2)) {
					*newStateId = modifiedPFullState;
					UpdateStateHistory(MAX_PACKETID);
				}
				else if (modifiedPFullState > modifiedLastState || lastState == 0)
					*newStateId = modifiedPFullState;
				else
					return false;
			}
			return true;
		}
	};
}

#endif //INETWORKDELTACOMPONENT_H
