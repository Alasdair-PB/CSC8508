//
// Contributors: Alasdair
//

#include "INetworkComponent.h"
using namespace NCL::CSC8508;
using namespace NCL;
using namespace CSC8508;

INetworkComponent::INetworkComponent(int objId, int ownId, int componentId, bool clientOwned)
{
	this->objectID = objId;
	this->ownerID = ownId;
	this->clientOwned = clientOwned;
	this->componentID = componentId;
}
