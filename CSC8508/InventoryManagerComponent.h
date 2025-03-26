#pragma once
#ifndef INVENTORY_MANAGER_COMPONENT_H
#define INVENTORY_MANAGER_COMPONENT_H

#include "IComponent.h"
#include "ItemComponent.h"
#include <vector>

namespace NCL {
    namespace CSC8508 {

        class InventoryManagerComponent : public IComponent {
        public:
            InventoryManagerComponent(GameObject& gameObject, int maxStorage, float itemCarryOffset, float itemDropOffset)
                : IComponent(gameObject), transform(gameObject.GetTransform()), 
                itemCarryOffset(itemCarryOffset), itemDropOffset(itemDropOffset){
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
                objectTransform.SetPosition(transform.GetPosition() + 
                    Vector3(0,carryYOffset,0) + 
                    (GetDirection() * itemCarryOffset));
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

            int maxItemStorage;
            int scrollIndex = 0;
            float itemCarryOffset;
            float itemDropOffset;
            float carryYOffset = 3;
            Transform& transform;
            
            std::vector<ItemComponent*> storedItems;
             bool ItemAtScrollIndex() { return storedItems.size() > scrollIndex; }

             Vector3 GetDirection() {
                 return (transform.GetOrientation() * Vector3(0, 0, 1));
             }

            void ReturnItemToWorld(int inventoryIndex) {
                Transform& objectTransform = storedItems[inventoryIndex]->GetGameObject().GetTransform();
                objectTransform.SetPosition(transform.GetPosition() + GetDirection() * itemDropOffset);
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
    }
}

#endif // INVENTORY_MANAGER_COMPONENT_H
