#pragma once
#include "IComponent.h"



namespace NCL {
    namespace CSC8508 {

        class StaminaComponent : public IComponent {
        public:
            StaminaComponent(GameObject& gameObject, float initStam, float maxStam): IComponent(gameObject) {
                maxStamina = std::max(1.0f, maxStam);
                stamina = std::min(initStam, maxStamina);
            }
            ~StaminaComponent() = default;

            void Update(float dt)override {
                stamina = std::min(stamina + sRegen * dt, maxStamina);
            }

            void IncreaseStamina(float regen) {
                stamina = std::min(stamina + abs(regen), maxStamina);
            }
            void DecreaseStamina(float regen) {
                stamina = std::max(stamina - abs(regen), 0.0f);
            }

            void SetStaminaAction(uint32_t a, float s) {
                staminaActionMap[a] = s;
            }

            bool CanPerformAction(uint32_t a) {
                if (!staminaActionMap.contains(a)) { return false; }
                if (stamina - staminaActionMap[a] < 0.0f) { return false; }
                return true;
            }

            bool PerformActionIfAble(uint32_t a) {
                if (CanPerformAction(a)) {
                    stamina -= staminaActionMap[a];
                    return true;
                }
                return false;
            }

            void SetStamina(float s) {
                stamina = std::min(stamina, maxStamina);
            }
            void SetStaminaRegain(float r) {
                sRegen = r;
            }

            void SetMaxStamina(float m) {
                maxStamina = m;
            }

            float GetMaxStamina() {
                return maxStamina;
            }

            float GetStamina() {
                return stamina;
            }

        protected:
            float stamina;
            float sRegen;
            float maxStamina;

            std::unordered_map<uint32_t, float> staminaActionMap;
        };
    }
}