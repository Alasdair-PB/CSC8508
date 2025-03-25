#pragma once
#ifndef INVENTORY_NETWORKMANAGER_COMPONENT_H
#define INVENTORY_NETWORKMANAGER_COMPONENT_H

#include "IComponent.h"
#include "INetworkComponent.h"
#include "INetworkDeltaComponent.h"
#include "ItemComponent.h"
#include "InventoryManagerComponent.h"
#include "ComponentManager.h"
#include <vector>
#include "../CSC8508CoreClasses/TransformNetworkComponent.h"

namespace NCL::CSC8508 {

	const int MAX_INVENTORY_ITEMS = 5;

	class InventoryNetworkState : public INetworkState {
	public:
		InventoryNetworkState() {}
		~InventoryNetworkState() = default;
		int inventory[MAX_INVENTORY_ITEMS];
	};

	struct FullPacket : public IFullNetworkPacket {
	public:
		InventoryNetworkState fullState;
		FullPacket() {
			type = Full_State;
			size = sizeof(FullPacket) - sizeof(GamePacket);
		}
	};

	class InventoryNetworkManagerComponent : public InventoryManagerComponent, public INetworkDeltaComponent {
	public:
		InventoryNetworkManagerComponent(GameObject& gameObject, int maxStorage, int objId, int ownId, int componId, bool clientOwned)
			:
			InventoryManagerComponent(gameObject, maxStorage),
			INetworkDeltaComponent(objId, ownId, componId, clientOwned, new InventoryNetworkState()) {
		}

		virtual std::unordered_set<std::type_index>& GetDerivedTypes() const override {
			static std::unordered_set<std::type_index> types = {
				std::type_index(typeid(IComponent)),
				std::type_index(typeid(INetworkComponent)),
				std::type_index(typeid(InventoryManagerComponent)),
				std::type_index(typeid(INetworkDeltaComponent))
			};
			return types;
		}

		vector<GamePacket*> WritePacket() {
			vector<GamePacket*> packets;
			FullPacket* fp = new FullPacket();
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

		vector<GamePacket*> WriteDeltaPacket(bool* deltaFrame) override { return WritePacket(); }
		vector<GamePacket*> WriteFullPacket() override { return WritePacket(); }

	private:

		bool ReadPacket() {}
		bool ReadDeltaPacket(IDeltaNetworkPacket& idp) override { return false; }

		bool ReadFullPacket(IFullNetworkPacket& ifp) override {
			int newStateId = 0;
			FullPacket p = ((FullPacket&)ifp);

			if (!UpdateFullStateHistory<InventoryNetworkState, FullPacket>(p, &newStateId))
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
					ComponentManager::OperateOnBufferContents<TransformNetworkComponent>(
						[&lastInvFullState, &itemDelta, &i](TransformNetworkComponent* o) {
							if (o->GetObjectID() == lastInvFullState->inventory[i]) {
								ItemComponent* itemComponent = o->GetGameObject().TryGetComponent<ItemComponent>();
								if (itemComponent) {
									itemDelta = itemComponent;
								}
							}
						}
					);

					// Maybe for every inventory component drop the item if carried?

					if (itemDelta != nullptr)
						storedItems[i] = itemDelta;
				}
			}
			return true;
		}
		bool ReadEventPacket(INetworkPacket& p) override { return true; }
    };
};

#endif // INVENTORY_MANAGER_COMPONENT_H
