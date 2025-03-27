//
// Contributors: Alasdair
//

#include "INetworkComponent.h"
using namespace NCL::CSC8508;
using namespace NCL;
using namespace CSC8508;

INetworkComponent::INetworkComponent(int objId, int ownId, int componentId, int pFabID, bool clientOwned)
{
	this->objectID = objId;
	this->ownerID = ownId;
	this->clientOwned = clientOwned;
	this->pfabID = pFabID;
	this->componentID = componentId;
}
