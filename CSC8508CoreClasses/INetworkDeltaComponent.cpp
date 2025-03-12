//
// Contributors: Alasdair
//

#include "INetworkDeltaComponent.h"
using namespace NCL::CSC8508;
using namespace NCL;
using namespace CSC8508;

INetworkDeltaComponent::INetworkDeltaComponent(int objId, int ownId, int componentId, bool clientOwned, INetworkState* state)
	: INetworkComponent(objId, ownId, componentId, clientOwned),
	deltaErrors(0), fullErrors(0), lastFullState(state) {}

vector<GamePacket*> INetworkDeltaComponent::WriteDeltaFullPacket(bool deltaFrame){
	if (deltaFrame) {
		bool foundDelta = true;
		auto packets = WriteDeltaPacket(&foundDelta);
		return foundDelta ? packets : WriteFullPacket();
	}
	return WriteFullPacket();
}

bool INetworkDeltaComponent::ReadDeltaFullPacket(INetworkPacket& p)
{
	if (p.type == Delta_State)
		return ReadDeltaPacket((IDeltaNetworkPacket&)p);
	if (p.type == Full_State)
		return ReadFullPacket((IFullNetworkPacket&)p);
	return false;
}

bool INetworkDeltaComponent::ReadDeltaPacketState(IDeltaNetworkPacket& p)
{
	if (p.fullID != lastFullState->stateID)
		return false;
	return ReadDeltaPacket(p);
}


bool INetworkDeltaComponent::ReadFullPacketState(IFullNetworkPacket& p) {
	if (p.fullState.stateID < lastFullState->stateID) 
		return false;
	return ReadFullPacket(p);
}

void INetworkDeltaComponent::UpdateStateHistory(int minID) {
	for (auto i = stateHistory.begin(); i < stateHistory.end();) {
		if ((*i)->stateID < minID) {
			delete* i;
			i = stateHistory.erase(i);
		}
		else
			++i;
	}
}