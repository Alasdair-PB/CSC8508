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
		ComponentManager::OperateOnBufferContents<InventoryNetworkManagerComponent>(
			[&itemDelta](InventoryNetworkManagerComponent* o) { o->RemoveItemEntry(itemDelta); });
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
	if (p.packetSubType == Sell)
		return ReadSellInventoryPacket((SellInventoryPacket&)p);
	else if (p.packetSubType == Deposit)
		return ReadDepositWalletPacket((DepositWalletPacket&)p);
	return false;
}


// Worthing coming back to this and determining
// if this needs to do anything on non-owners as it will 
// will get set on network events or will be rolled back on missed packets
// This may be fighting with itself for no reason
void InventoryNetworkManagerComponent::DepositWalletToQuota(){
	if (IsOwner()) {
		InventoryManagerComponent::DepositWalletToQuota();
		DepositWalletPacket* pck = new DepositWalletPacket();
		pck->despoited = deposited;
		pck->wallet = wallet;
		SendEventPacket(pck);
		delete pck;
	}
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
		return InventoryManagerComponent::SellAllItems();
	}
	return 0;
}

// Will need a revision if Deposit allows for variable amounts instead of only full despoit
bool InventoryNetworkManagerComponent::ReadDepositWalletPacket(DepositWalletPacket pck)
{
	wallet = pck.wallet;
	deposited = pck.despoited;
	return true;
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
	wallet = p.fullState.wallet;
	deposited = p.fullState.deposited;
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
	state->deposited = deposited;
	fp->fullState.deposited = deposited;

	fp->fullState.stateID = state->stateID;
	if (clientOwned && state->stateID >= MAX_PACKETID)
		state->stateID = 0;

	SetPacketOwnership(fp);
	packets.push_back(fp);
	return packets;
}
