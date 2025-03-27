#include "InventoryNetworkManagerComponent.h"
#include "ComponentManager.h"
#include "../CSC8508CoreClasses/FullTransformNetworkComponent.h"
using namespace NCL;
using namespace CSC8508;

void InventoryNetworkManagerComponent::TrySetStoredItems(InventoryNetworkState* lastInvFullState, int i) {
	ItemComponent* itemDelta = nullptr;
	ComponentManager::OperateOnBufferContents<FullTransformNetworkComponent>(
		[&lastInvFullState, &itemDelta, &i](FullTransformNetworkComponent* o) {
			if (o->GetObjectID() == lastInvFullState->inventory[i]) {
				ItemComponent* itemComponent = o->GetGameObject().TryGetComponent<ItemComponent>();
				if (itemComponent)
					itemDelta = itemComponent;
			}
		}
	);
	if (itemDelta != nullptr) {
		ComponentManager::OperateOnBufferContents<InventoryManagerComponent>(
			[&itemDelta](InventoryManagerComponent* o) { o->RemoveItemEntry(itemDelta);});
		if (i < storedItems.size())
			storedItems[i] = itemDelta;
		else
			PushItemToInventory(itemDelta);
	}
}

bool InventoryNetworkManagerComponent::InventoryIsMatch(SellInventoryPacket& pck) {
	// Not implemented
	return true;
}

void InventoryNetworkManagerComponent::DisableSoldItemInWorld(SellInventoryPacket& pck, int i) {
	InventoryNetworkManagerComponent* self = this;
	int componentId = pck.soldInventory[i];
	ComponentManager::OperateOnBufferContents<FullTransformNetworkComponent>(
		[&componentId, &self](FullTransformNetworkComponent* o) {
			if (o->GetObjectID() == componentId) {
				o->GetGameObject().SetEnabled(false);
				ItemComponent* itemComponent = o->GetGameObject().TryGetComponent<ItemComponent>();
				self->DisableItemInWorld(itemComponent);
			}
		}
	);
}

void InventoryNetworkManagerComponent::DisableSoldItemsInWorld(SellInventoryPacket& pck) {
	for (int i = 0; i < MAX_INVENTORY_ITEMS; i++) {
		if (pck.soldInventory[i] == 0)
			continue;
		DisableSoldItemInWorld(pck, i);
	}
}

bool InventoryNetworkManagerComponent::ReadEventPacket(INetworkPacket& p) {
	if (p.packetSubType == None)
		return ReadSellInventoryPacket((SellInventoryPacket&)p);
	return false;
}

float InventoryNetworkManagerComponent::SellAllItems() {
	if (IsOwner()) {
		SellInventoryPacket* pck = new SellInventoryPacket();
		for (int i = 0; i < maxItemStorage; i++) {
			pck->soldInventory[i] = 0;
			if (storedItems.size() > i) {
				FullTransformNetworkComponent* networkComponent = 
					storedItems[i]->GetGameObject().TryGetComponent<FullTransformNetworkComponent>();
				if (networkComponent != nullptr) {
					int id = networkComponent->GetObjectID();
					pck->soldInventory[i] = id;
				}
			}
		}
		SendEventPacket(pck);
		delete pck;
	}	
	return InventoryManagerComponent::SellAllItems();
}

bool InventoryNetworkManagerComponent::ReadSellInventoryPacket(SellInventoryPacket pck)
{
	bool soldInventory = false; 
	int count = 0;

	for (int i = 0; i < MAX_INVENTORY_ITEMS; i++)
		if (i != 0) count++;
	if (count != storedItems.size())
		DisableSoldItemsInWorld(pck);
	else {
		// Presumes inventory is the same
		if (storedItems.size() > 0) SellAllItems();
		else DisableSoldItemsInWorld(pck);
	}
	return true;
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

		if (i < storedItems.size() && storedItems[i]) {
			FullTransformNetworkComponent* networkComponent = 
				storedItems[i]->GetGameObject().TryGetComponent<FullTransformNetworkComponent>();
			if (!networkComponent) continue;
			if (networkComponent->GetObjectID() == lastInvFullState->inventory[i])
				continue;
			TrySetStoredItems(lastInvFullState, i);
		}
		else
			TrySetStoredItems(lastInvFullState, i);
	}

	if (wallet != p.fullState.wallet)
		std::cout << "Rollback Sold::" << p.fullState.wallet << std::endl;
	wallet = p.fullState.wallet;
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
				FullTransformNetworkComponent* networkComponent = 
					storedItems[i]->GetGameObject().TryGetComponent<FullTransformNetworkComponent>();
				if (networkComponent != nullptr) {
					int id = networkComponent->GetObjectID();
					state->inventory[i] = id;
					fp->fullState.inventory[i] = id;
				}
			}
		}
	}
	state->wallet = wallet;
	fp->fullState.wallet = wallet;
	fp->fullState.stateID = state->stateID;
	if (clientOwned && state->stateID >= MAX_PACKETID)
		state->stateID = 0;

	SetPacketOwnership(fp);
	packets.push_back(fp);
	return packets;
}
