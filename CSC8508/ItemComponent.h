#ifndef ITEM_COMPONENT_H
#define ITEM_COMPONENT_H

#include "../CSC8508CoreClasses/IComponent.h"
#include "DamageableComponent.h"
#include "../Event/EventManager.h"
#include "../Event/Event.h"

namespace NCL {
    namespace CSC8508 {

        class ItemComponent : public IComponent , public EventListener<DeathEvent> {
        private:
            int saleValue;
            vector<IComponent*> disabledOnPickup;

        public:
            ItemComponent(GameObject& gameObject, int initialSaleValue)
                : IComponent(gameObject), saleValue(initialSaleValue) {
                EventManager::RegisterListener<DeathEvent>(this);
            }

            ~ItemComponent() {
                disabledOnPickup.clear();
            }

            void OnAwake() override {
                EventManager::RegisterListener<DeathEvent>(this);
            }

            int GetSaleValue() const {
                return saleValue;
            }

            void SetSaleValue(int value) {
                saleValue = std::max(0, value);
            }

            void OnEvent(DeathEvent* event) override {
                if (&event->GetGameObject() == &GetGameObject())
                    SetSaleValue(0);
            }

            void DisableIComponentOnPickup(IComponent* component) {
                disabledOnPickup.push_back(component);
            }

            void SetEnabledComponentStates(bool state) {
                for (IComponent* icom : disabledOnPickup)
                    icom->SetEnabled(state);
            }

            /// <summary>
            /// IComponent Save data struct definition
            /// </summary>
            struct ItemComponentDataStruct;

            void Load(std::string assetPath, size_t allocationStart) override;
            size_t Save(std::string assetPath, size_t* allocationStart) override;
        };
    }
}


#endif // ITEM_COMPONENT_H
