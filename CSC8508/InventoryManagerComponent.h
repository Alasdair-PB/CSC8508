#pragma once
#ifndef INVENTORY_MANAGER_COMPONENT_H
#define INVENTORY_MANAGER_COMPONENT_H

#include "IComponent.h"
#include "ItemComponent.h"
#include <vector>

namespace NCL::CSC8508 {

    class InventoryManagerComponent : public IComponent {
    private:
        std::vector<GameObject*> storedItems;
        int maxItemStorage;

    public:
        InventoryManagerComponent(GameObject& gameObject, int maxStorage)
            : IComponent(gameObject), maxItemStorage(maxStorage) {
            storedItems.reserve(maxItemStorage);
        }

        bool PushItemToInventory(GameObject* object) {
            if (!object || !object->HasComponent<ItemComponent>()) return false;
            if (storedItems.size() >= maxItemStorage) return false;

            
            // Instead of directly using `SetEnable(false)`, adjust physics properties.

            object->SetPhysicsEnabled(false);
            object->SetVisible(false);

            storedItems.push_back(object);
            return true;
        }

        GameObject* PopItemFromInventory(int inventoryIndex) {
            if (inventoryIndex < 0 || inventoryIndex >= storedItems.size()) return nullptr;

            GameObject* item = storedItems[inventoryIndex];
            storedItems.erase(storedItems.begin() + inventoryIndex);

            
            item->SetPhysicsEnabled(true);
            item->SetVisible(true);

            return item;
        }

        void SellAllItems() {
            for (GameObject* item : storedItems) {
                if (item->HasComponent<ItemComponent>()) {
                    ItemComponent* itemComp = item->GetComponent<ItemComponent>();
                    Game::GetInstance().AddMoney(itemComp->GetSaleValue());
                }
            }
            storedItems.clear();
        }

        /// <summary>
        /// IComponent Save data struct definition
        /// </summary>
        struct InventoryManagerComponentDataStruct;

        void Load(std::string assetPath, size_t allocationStart) override;
        size_t Save(std::string assetPath, size_t* allocationStart) override;
    };

    struct InventoryManagerComponent::InventoryManagerComponentDataStruct : public ISerializedData {
        InventoryManagerComponentDataStruct() : maxItemStorage(0) {}
        InventoryManagerComponentDataStruct(int maxItemStorage, std::vector<GameObject*> items)
            : maxItemStorage(maxItemStorage), storedItems(items) {}

        int maxItemStorage;
        std::vector<GameObject*> storedItems;

        static auto GetSerializedFields() {
            return std::make_tuple(
                SERIALIZED_FIELD(InventoryManagerComponentDataStruct, maxItemStorage)
            );
        }
    };

    size_t InventoryManagerComponent::Save(std::string assetPath, size_t* allocationStart) {
        InventoryManagerComponentDataStruct saveInfo(maxItemStorage, storedItems);
        SaveManager::GameData saveData = ISerializedData::CreateGameData<InventoryManagerComponentDataStruct>(saveInfo);
        return SaveManager::SaveGameData(assetPath, saveData, allocationStart, true);
    }

    void InventoryManagerComponent::Load(std::string assetPath, size_t allocationStart) {
        InventoryManagerComponentDataStruct loadedSaveData =
            ISerializedData::LoadISerializable<InventoryManagerComponentDataStruct>(assetPath, allocationStart);

        maxItemStorage = std::max(1, loadedSaveData.maxItemStorage);
    }

};

#endif // INVENTORY_MANAGER_COMPONENT_H
