#ifndef ITEM_COMPONENT_H
#define ITEM_COMPONENT_H

#include "IComponent.h"
#include "DamageableComponent.h"
#include "EventManager.h"
#include "Event.h"

namespace NCL::CSC8508 {

    class ItemComponent : public IComponent {
    private:
        int saleValue;
        DamageableComponent* damageable;

    public:
        ItemComponent(GameObject& gameObject, int initialSaleValue)
            : IComponent(gameObject), saleValue(initialSaleValue), damageable(nullptr) {
            if (gameObject.HasComponent<DamageableComponent>()) {
                damageable = gameObject.GetComponent<DamageableComponent>();
                EventManager::Subscribe<DeathEvent>(this, &ItemComponent::OnDeath);
            }
        }

        ~ItemComponent() {
            EventManager::Unsubscribe<DeathEvent>(this, &ItemComponent::OnDeath);
        }

        int GetSaleValue() const {
            return saleValue;
        }

        void SetSaleValue(int value) {
            saleValue = std::max(0, value);
        }

        void OnDeath(DeathEvent* event) {
            if (&event->GetGameObject() == &GetGameObject()) {
                SetSaleValue(0); // **物品损坏，售价归零**
            }
        }

        /// <summary>
        /// IComponent Save data struct definition
        /// </summary>
        struct ItemComponentDataStruct;

        void Load(std::string assetPath, size_t allocationStart) override;
        size_t Save(std::string assetPath, size_t* allocationStart) override;
    };

    struct ItemComponent::ItemComponentDataStruct : public ISerializedData {
        ItemComponentDataStruct() : saleValue(0) {}
        ItemComponentDataStruct(int saleValue) : saleValue(saleValue) {}

        int saleValue;

        static auto GetSerializedFields() {
            return std::make_tuple(
                SERIALIZED_FIELD(ItemComponentDataStruct, saleValue)
            );
        }
    };

    size_t ItemComponent::Save(std::string assetPath, size_t* allocationStart) {
        ItemComponentDataStruct saveInfo(saleValue);
        SaveManager::GameData saveData = ISerializedData::CreateGameData<ItemComponentDataStruct>(saveInfo);
        return SaveManager::SaveGameData(assetPath, saveData, allocationStart, true);
    }

    void ItemComponent::Load(std::string assetPath, size_t allocationStart) {
        ItemComponentDataStruct loadedSaveData = ISerializedData::LoadISerializable<ItemComponentDataStruct>(assetPath, allocationStart);
        saleValue = std::max(0, loadedSaveData.saleValue);
    }

};

#endif // ITEM_COMPONENT_H
