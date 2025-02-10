//
// Contributors: Alasdair
//

#include "INetworkComponent.h"
using namespace NCL::CSC8508;
using namespace NCL;
using namespace CSC8508;

INetworkComponent::INetworkComponent(int objId, int ownId, bool clientOwned)
{
	objectID = objId;
	ownerID = ownId;
	clientOwned = clientOwned;
}

INetworkComponent::~INetworkComponent() {}

vector<GamePacket*> INetworkComponent::WritePacket(bool deltaFrame, int stateID){
	if (deltaFrame) {
		bool foundDelta = true;
		auto packets = WriteDeltaPacket(&foundDelta, stateID);
		return foundDelta ? packets : WriteFullPacket();
	}
	return WriteFullPacket();
}


void INetworkComponent::UpdateStateHistory(int minID) {
	for (auto i = stateHistory.begin(); i < stateHistory.end();) {
		if ((*i).stateID < minID)
			i = stateHistory.erase(i);
		else
			++i;
	}
}

NetworkState& INetworkComponent::GetLatestNetworkState() {
	return lastFullState;
}

bool INetworkComponent::GetNetworkState(int stateID, NetworkState& state) {
	for (auto i = stateHistory.begin(); i < stateHistory.end(); ++i) {
		if ((*i).stateID == stateID) {
			state = (*i);
			return true;
		}
	}
	return false;
}
