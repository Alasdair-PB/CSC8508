#pragma once
#ifndef INVENTORY_MANAGER_COMPONENT_H
#define INVENTORY_MANAGER_COMPONENT_H

#include "IComponent.h"
#include "ItemComponent.h"

#include "UISystem.h"
#include "InventoryUI.h"
#include <vector>

namespace NCL {
    namespace CSC8508 {

        class InventoryManagerComponent : public IComponent {
        public:
            InventoryManagerComponent(GameObject& gameObject, int maxStorage, float itemCarryOffset, float itemDropOffset)
                : IComponent(gameObject), transform(gameObject.GetTransform()), 
                itemCarryOffset(itemCarryOffset), itemDropOffset(itemDropOffset), wallet(0.0f), deposited(0.0f)
            {
                maxItemStorage = std::max(1, maxStorage);
            }

            virtual void InitInventoryUI() {
                UI::UISystem::GetInstance()->PushNewStack(inventoryUI->inventoryUI, "Inventory");
                inventoryUI->PushInventoryElement(InventoryMenu());
            }

            virtual void OnAwake() override
            {
                InitInventoryUI();
            }

            ~InventoryManagerComponent() = default;

            bool PushItemToInventory(ItemComponent* item) {
                if (!item) return false;
                if (storedItems.size() >= maxItemStorage) return false;

                item->SetEnabledComponentStates(false);
                storedItems.push_back(item);
                item->GetGameObject().SetEnabled(true);
                scrollIndex = static_cast<int>(storedItems.size()) - 1;
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

            void DisableItemInWorld(ItemComponent* item) {
                item->SetSaleValue(0);
                item->GetGameObject().SetEnabled(false);
            }

            int GetQuota();

            std::function<CSC8508::PushdownState::PushdownResult()> InventoryMenu() {
                std::function<CSC8508::PushdownState::PushdownResult()> func = [this]() -> CSC8508::PushdownState::PushdownResult {
                    int index = 0;
                    ImGui::Text(("Wallet: " + std::to_string(wallet)).c_str());

                    for (auto const& item : storedItems) {
                        ImGui::SetCursorPosY(0.04f * Window::GetWindow()->GetScreenSize().y);
                        std::string name = item->GetName();
                        std::string sellVal = std::to_string(item->GetSaleValue());
                        if (index == scrollIndex) {
                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.00f, 0.60f, 0.00f, 1.00f));
                            ImGui::Text(("Item: " + name).c_str());
                            ImGui::Text(("Sell Value: " + sellVal).c_str());
                            ImGui::PopStyleColor();
                        }
                        else {
                            ImGui::Text(("Item: " + name).c_str());
                            ImGui::Text(("Sell Value: " + sellVal).c_str());
                        }
                    }
                    ImGui::Text(("DepositedFunds: " + std::to_string(deposited)).c_str());
                    ImGui::Text(("Global Quota: " + std::to_string(GetQuota())).c_str());

                    index += 1;
                    return CSC8508::PushdownState::PushdownResult::NoChange;
                    };
                return func;
            }

           virtual float SellAllItems() {
                float itemTotal = 0;
                for (ItemComponent* item : storedItems) {
                    itemTotal += item->GetSaleValue();
                    DisableItemInWorld(item);
                }
                storedItems.clear();
                wallet += itemTotal;
                return itemTotal;
            }

           float ResetWallet() {
			   float walletValue = wallet;
			   wallet = 0;
			   return walletValue;
		   }

           float GetWallet() {
			   return wallet;
           }

           float GetItemCombinedWeight() {
               float itemCombinedWeight = 0.0f;
               for (ItemComponent* item : storedItems)
                   itemCombinedWeight += item->GetItemWeight();
               return itemCombinedWeight;
           }

            /// <summary>
            /// IComponent Save data struct definition
            /// </summary>
            struct InventoryManagerComponentDataStruct;

            void Load(std::string assetPath, size_t allocationStart) override;
            size_t Save(std::string assetPath, size_t* allocationStart) override;
            
            void RemoveItemEntry(ItemComponent* item) {
                for (int i = 0; i < storedItems.size(); i++) {
                    if (storedItems[i] == item) {
                        RemoveItemEntry(i);
                        return;
                    }
                }
            }

            void RemoveItemEntry(int inventoryIndex) {
                storedItems.erase(storedItems.begin() + inventoryIndex);
            }

            virtual void DepositWalletToQuota() {
				deposited += wallet;
				wallet = 0;
            }

            float GetDepositedSum() { return deposited; }

        protected:
            int maxItemStorage;
            int scrollIndex = 0;
            float itemCarryOffset;
            float itemDropOffset;
            float carryYOffset = 2.2f;
            float wallet; 
            float deposited;
            Transform& transform;

            UI::InventoryUI* inventoryUI = new UI::InventoryUI;
            
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
                RemoveItemEntry(inventoryIndex);
                return item;
            }
        };
    }
}

#endif // INVENTORY_MANAGER_COMPONENT_H
