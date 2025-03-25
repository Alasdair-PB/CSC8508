#pragma once
#ifndef INVENTORY_MANAGER_COMPONENT_H
#define INVENTORY_MANAGER_COMPONENT_H

#include "IComponent.h"
#include "ItemComponent.h"
#include <vector>

namespace NCL::CSC8508 {

    class InventoryManagerComponent : public IComponent {
    public:
        InventoryManagerComponent(GameObject& gameObject, int maxStorage)
            : IComponent(gameObject), transform(gameObject.GetTransform()) {
            maxItemStorage = std::max(1, maxStorage);
        }

        bool PushItemToInventory(ItemComponent* item) {
            if (!item) return false;
            if (storedItems.size() >= maxItemStorage) return false;

            item->SetEnabledComponentStates(false);
            storedItems.push_back(item);
            item->GetGameObject().SetEnabled(true);
            scrollIndex = storedItems.size() - 1;
            return true;
        }

        bool ItemInHand() {
            return ItemAtScrollIndex();
        }

        void Update(float deltaTime) override {
            if (!ItemAtScrollIndex()) return;
            Transform& objectTransform = storedItems[scrollIndex]->GetGameObject().GetTransform();
            objectTransform.SetPosition(transform.GetPosition() + itemCarryOffset);
        }

        void IncrementScrollIndex() {
            if (ItemAtScrollIndex()) storedItems[scrollIndex]->GetGameObject().SetEnabled(false);
            scrollIndex >= maxItemStorage - 1 ? scrollIndex = 0 : scrollIndex++;

            if (!ItemAtScrollIndex()) return;
            storedItems[scrollIndex]->GetGameObject().SetEnabled(true);
        }

        void DropItem() {
            if (!ItemAtScrollIndex()) return;
            PopItemFromInventory(scrollIndex);
        }

        void DropItemAtIndex(int inventoryIndex) {
            PopItemFromInventory(inventoryIndex);
        }

        float SellAllItems() {
            float itemTotal = 0;
            for (ItemComponent* item : storedItems) {
                itemTotal += item->GetSaleValue();
                item->GetGameObject().SetEnabled(false);
            }
            storedItems.clear();
        }

        /// <summary>
        /// IComponent Save data struct definition
        /// </summary>
        struct InventoryManagerComponentDataStruct;

        void Load(std::string assetPath, size_t allocationStart) override;
        size_t Save(std::string assetPath, size_t* allocationStart) override;

    protected:
        std::vector<ItemComponent*> storedItems;
        int maxItemStorage;
        int scrollIndex = 0;
        Vector3 itemCarryOffset;
        Vector3 itemDropOffset;
        Transform& transform;

        bool ItemAtScrollIndex() { return storedItems.size() >= scrollIndex; }

        void ReturnItemToWorld(int inventoryIndex) {
            Transform& objectTransform = storedItems[inventoryIndex]->GetGameObject().GetTransform();
            objectTransform.SetPosition(transform.GetPosition() + itemDropOffset);
        }

        ItemComponent* PopItemFromInventory(int inventoryIndex) {
            if (inventoryIndex < 0 || inventoryIndex >= storedItems.size()) return nullptr;
            ItemComponent* item = storedItems[inventoryIndex];
            item->SetEnabledComponentStates(true);
            item->GetGameObject().SetEnabled(true);
            ReturnItemToWorld(inventoryIndex);

            storedItems.erase(storedItems.begin() + inventoryIndex);
            return item;
        }

    };

    struct InventoryManagerComponent::InventoryManagerComponentDataStruct : public ISerializedData {
        InventoryManagerComponentDataStruct() : maxItemStorage(0) {}
        InventoryManagerComponentDataStruct(int maxItemStorage, Vector3 itemCarryOffset, Vector3 itemDropOffset)
            : maxItemStorage(maxItemStorage), itemCarryOffset(itemCarryOffset), itemDropOffset(itemDropOffset){}

        int maxItemStorage;
        Vector3 itemCarryOffset;
        Vector3 itemDropOffset;

        static auto GetSerializedFields() {
            return std::make_tuple(
                SERIALIZED_FIELD(InventoryManagerComponentDataStruct, maxItemStorage),
                SERIALIZED_FIELD(InventoryManagerComponentDataStruct, itemCarryOffset),
                SERIALIZED_FIELD(InventoryManagerComponentDataStruct, itemDropOffset)
            );
        }
    };

    size_t InventoryManagerComponent::Save(std::string assetPath, size_t* allocationStart) {
        InventoryManagerComponentDataStruct saveInfo(maxItemStorage, itemCarryOffset, itemDropOffset);
        SaveManager::GameData saveData = ISerializedData::CreateGameData<InventoryManagerComponentDataStruct>(saveInfo);
        return SaveManager::SaveGameData(assetPath, saveData, allocationStart, true);
    }

    void InventoryManagerComponent::Load(std::string assetPath, size_t allocationStart) {
        InventoryManagerComponentDataStruct loadedSaveData =
            ISerializedData::LoadISerializable<InventoryManagerComponentDataStruct>(assetPath, allocationStart);

        maxItemStorage = std::max(1, loadedSaveData.maxItemStorage);
        itemCarryOffset = loadedSaveData.itemCarryOffset;
        itemDropOffset = loadedSaveData.itemDropOffset;
    }
};

#endif // INVENTORY_MANAGER_COMPONENT_H
