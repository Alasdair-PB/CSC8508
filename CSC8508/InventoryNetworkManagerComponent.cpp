#include "InventoryNetworkManagerComponent.h"
//#include "ComponentManager.h"
#include "../CSC8508CoreClasses/TransformNetworkComponent.h"
using namespace NCL;
using namespace CSC8508;

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

		if (storedItems[i]) {
			TransformNetworkComponent* networkComponent = storedItems[i]->GetGameObject().TryGetComponent<TransformNetworkComponent>();

			if (!networkComponent) continue;

			if (networkComponent->GetObjectID() == lastInvFullState->inventory[i])
				continue;

			ItemComponent* itemDelta = nullptr;
			/*ComponentManager::OperateOnBufferContents<TransformNetworkComponent>(
				[&lastInvFullState, &itemDelta, &i](TransformNetworkComponent* o) {
					if (o->GetObjectID() == lastInvFullState->inventory[i]) {
						ItemComponent* itemComponent = o->GetGameObject().TryGetComponent<ItemComponent>();
						if (itemComponent) {
							itemDelta = itemComponent;
						}
					}
				}
			);*/

			// Maybe for every inventory component drop the item if carried?
			if (itemDelta != nullptr)
				storedItems[i] = itemDelta;
		}
	}
	return true;
}



vector<GamePacket*> InventoryNetworkManagerComponent::WritePacket() {
	vector<GamePacket*> packets;
	InvFullPacket* fp = new InvFullPacket();
	InventoryNetworkState* state = static_cast<InventoryNetworkState*>(lastFullState);

	for (int i = 0; i < MAX_INVENTORY_ITEMS; i++) {
		state->inventory[i] = 0;
		if (i < storedItems.size()) {
			if (storedItems[i] != nullptr) {
				TransformNetworkComponent* networkComponent = storedItems[i]->GetGameObject().TryGetComponent<TransformNetworkComponent>();
				if (networkComponent != nullptr)
					state->inventory[i] = networkComponent->GetObjectID();
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
