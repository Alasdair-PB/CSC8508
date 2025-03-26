#include "InventoryNetworkManagerComponent.h"
#include "ComponentManager.h"
#include "../CSC8508CoreClasses/TransformNetworkComponent.h"
using namespace NCL;
using namespace CSC8508;


void InventoryNetworkManagerComponent::TrySetStoredItems(InventoryNetworkState* lastInvFullState, int i) {
	ItemComponent* itemDelta = nullptr;
	std::cout << "Searching for matching item" << std::endl;

	ComponentManager::OperateOnBufferContents<TransformNetworkComponent>(
		[&lastInvFullState, &itemDelta, &i](TransformNetworkComponent* o) {
			if (o->GetObjectID() == lastInvFullState->inventory[i]) {
				std::cout << "matching componentId" << std::endl;
				ItemComponent* itemComponent = o->GetGameObject().TryGetComponent<ItemComponent>();
				if (itemComponent) {
					itemDelta = itemComponent;
				}
			}
		}
	);

	// Maybe for every inventory component drop the item if carried?
	// Add the item don't set it!!!
	if (itemDelta != nullptr)
		storedItems[i] = itemDelta;
}

bool InventoryNetworkManagerComponent::ReadFullPacket(IFullNetworkPacket& ifp) {
	int newStateId = 0;
	InvFullPacket p = ((InvFullPacket&)ifp);

	if (!UpdateFullStateHistory<InventoryNetworkState, InvFullPacket>(p, &newStateId))
		return false;

	for (int i = 0; i < MAX_INVENTORY_ITEMS; i++) 
		((InventoryNetworkState*)lastFullState)->inventory[i] = p.fullState.inventory[i];
	
	((InventoryNetworkState*)lastFullState)->stateID = newStateId;
	InventoryNetworkState* lastInvFullState = static_cast<InventoryNetworkState*>(lastFullState);

	if (!lastInvFullState) return false;

	for (int i = 0; i < MAX_INVENTORY_ITEMS; i++) {

		if (lastInvFullState->inventory[i] == 0)
			continue;

		std::cout << "Non zero recieved" << std::endl;

		if (i < storedItems.size() && storedItems[i]) {
			TransformNetworkComponent* networkComponent = storedItems[i]->GetGameObject().TryGetComponent<TransformNetworkComponent>();
			if (!networkComponent) continue;
			if (networkComponent->GetObjectID() == lastInvFullState->inventory[i])
				continue;
			TrySetStoredItems(lastInvFullState, i);
		}
		else
			TrySetStoredItems(lastInvFullState, i);
	}
	return true;
}



vector<GamePacket*> InventoryNetworkManagerComponent::WritePacket() {
	vector<GamePacket*> packets;
	InvFullPacket* fp = new InvFullPacket();
	InventoryNetworkState* state = static_cast<InventoryNetworkState*>(lastFullState);

	for (int i = 0; i < MAX_INVENTORY_ITEMS; i++) {
		state->inventory[i] = 0;
		fp->fullState.inventory[i] = 0;
		if (i < storedItems.size() && storedItems.size() > 0) {
			if (storedItems[i] != nullptr) {
				std::cout << "try get" << std::endl;
				TransformNetworkComponent* networkComponent = storedItems[i]->GetGameObject().TryGetComponent<TransformNetworkComponent>();
				if (networkComponent != nullptr) {
					int id = networkComponent->GetObjectID();
					state->inventory[i] = id;
					fp->fullState.inventory[i] = id;
				}
			}
		}
	}
	fp->fullState.stateID = state->stateID;
	if (clientOwned && state->stateID >= MAX_PACKETID)
		state->stateID = 0;

	SetPacketOwnership(fp);
	packets.push_back(fp);
	return packets;
}
