#pragma once
#include "INetworkDeltaComponent.h"
#include "InventoryManagerComponent.h"

namespace NCL::CSC8508 {
	const int MAX_INVENTORY_ITEMS = 5;

	class InventoryNetworkState : public INetworkState {
	public:
		InventoryNetworkState() {}
		~InventoryNetworkState() = default;
		int inventory[MAX_INVENTORY_ITEMS];
	};

	struct InvFullPacket : public IFullNetworkPacket {
	public:
		InventoryNetworkState fullState;
		InvFullPacket() {
			type = Full_State;
			size = sizeof(InvFullPacket) - sizeof(GamePacket);
		}
	};

	class InventoryNetworkManagerComponent : public InventoryManagerComponent, public INetworkDeltaComponent {

	public:
		InventoryNetworkManagerComponent(GameObject& gameObject, int maxStorage, float carryOffset, float dropOffset, int objId, int ownId, int componId, bool clientOwned)
			:
			InventoryManagerComponent(gameObject, maxStorage, carryOffset, dropOffset),
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

		vector<GamePacket*> WritePacket();
		vector<GamePacket*> WriteDeltaPacket(bool* deltaFrame) override { return WritePacket(); }
		vector<GamePacket*> WriteFullPacket() override { return WritePacket(); }

	private:
		void TrySetStoredItems(InventoryNetworkState* lastInvFullState, int i);
		bool ReadPacket() {}
		bool ReadDeltaPacket(IDeltaNetworkPacket& idp) override { return false; }
		bool ReadFullPacket(IFullNetworkPacket& ifp) override;
		bool ReadEventPacket(INetworkPacket& p) override { return true; }
	};

}
