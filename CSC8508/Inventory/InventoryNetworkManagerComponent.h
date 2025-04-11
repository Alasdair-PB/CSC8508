#pragma once
#include "INetworkDeltaComponent.h"
#include "InventoryManagerComponent.h"

namespace NCL::CSC8508 {
	const int MAX_INVENTORY_ITEMS = 5;

	enum InventorySubPacketType {Sell, Deposit};

	class InventoryNetworkState : public INetworkState {
	public:
		InventoryNetworkState() : inventory{ 0,0,0,0,0 }, wallet(0.0f), deposited(0.0f) {}
		~InventoryNetworkState() = default;
		int inventory[MAX_INVENTORY_ITEMS];
		float wallet;
		float deposited;
	};

	struct InvFullPacket : public IFullNetworkPacket {
	public:
		InventoryNetworkState fullState;
		InvFullPacket() {
			type = Full_State;
			size = sizeof(InvFullPacket) - sizeof(GamePacket);
		}
	};

	struct SellInventoryPacket : INetworkPacket {
		int soldInventory[MAX_INVENTORY_ITEMS];
		SellInventoryPacket() : soldInventory{0,0,0,0,0} {
			type = Component_Event;
			packetSubType = Sell;
			size = sizeof(SellInventoryPacket) - sizeof(GamePacket);
		}
	};

	struct DepositWalletPacket : INetworkPacket {
		float wallet;
		float despoited;
		DepositWalletPacket() : wallet(0.0f), despoited(0.0f) {
			type = Component_Event;
			packetSubType = Deposit;
			size = sizeof(SellInventoryPacket) - sizeof(GamePacket);
		}
	};

	class InventoryNetworkManagerComponent : public InventoryManagerComponent, public INetworkDeltaComponent {

	public:
		InventoryNetworkManagerComponent(GameObject& gameObject, int maxStorage, float carryOffset, float dropOffset, int objId, int ownId, int componId, int pfabID, bool clientOwned)
			:
			InventoryManagerComponent(gameObject, maxStorage, carryOffset, dropOffset),
			INetworkDeltaComponent(objId, ownId, componId, pfabID, clientOwned, new InventoryNetworkState()) {}
		~InventoryNetworkManagerComponent() = default;

		virtual std::unordered_set<std::type_index>& GetDerivedTypes() const override {
			static std::unordered_set<std::type_index> types = {
				std::type_index(typeid(IComponent)),
				std::type_index(typeid(INetworkComponent)),
				std::type_index(typeid(InventoryManagerComponent)),
				std::type_index(typeid(InventoryNetworkManagerComponent)),
				std::type_index(typeid(INetworkDeltaComponent))
			};
			return types;
		}

		void InitInventoryUI() override {
			if (!IsOwner()) return;
			InventoryManagerComponent::InitInventoryUI();
		}

		void OnAwake() override
		{
			InitInventoryUI();
		}

		vector<GamePacket*> WritePacket();
		vector<GamePacket*> WriteDeltaPacket(bool* deltaFrame) override { return WritePacket(); }
		vector<GamePacket*> WriteFullPacket() override { return WritePacket(); }

	private:
		void TrySetStoredItems(InventoryNetworkState* lastInvFullState, int i);
		bool ReadPacket() {}
		bool ReadSellInventoryPacket(SellInventoryPacket pck);
		bool ReadDepositWalletPacket(DepositWalletPacket pck);
		bool ReadDeltaPacket(IDeltaNetworkPacket& idp) override { return false; }
		bool ReadFullPacket(IFullNetworkPacket& ifp) override;
		bool ReadEventPacket(INetworkPacket& p) override;
		void DisableSoldItemsInWorld(SellInventoryPacket& pck);
		void DisableSoldItemInWorld(SellInventoryPacket& pck, int i);
		bool InventoryIsMatch(SellInventoryPacket& pck);
		float SellAllItems() override;
		void DepositWalletToQuota() override;

	};

}
