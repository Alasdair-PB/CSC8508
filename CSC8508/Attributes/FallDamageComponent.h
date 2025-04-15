#pragma once
#include "IComponent.h"
#include "DamageableComponent.h"
#include "PhysicsComponent.h"
#include "PhysicsObject.h"

namespace NCL {
    namespace CSC8508 {

        class FallDamageComponent : public IComponent {
        public:
            FallDamageComponent(GameObject& gameObject, float Y, int D):IComponent(gameObject) {
                fallDamageVelocity = Y; //Set player fallDamageVelocity to 24.0f
                takingFallDamage = false;
                fallDamage = D;
                fallingTime = 0.0f;
            }
            ~FallDamageComponent() = default;

            void OnAwake() override {
                damageComponent = GetGameObject().TryGetComponent<DamageableComponent>();
                physObj = GetGameObject().TryGetComponent<PhysicsComponent>()->GetPhysicsObject();
            }

            void Update(float dt) override {
                float yVelocity = physObj->GetLinearVelocity().y;
                if (abs(yVelocity) < 0.1f) {
                    if (takingFallDamage && fallingTime > 0.2f) {
                        damageComponent->Damage(fallDamage);
                        
                    }
                    takingFallDamage = false;
                    fallingTime = 0.0f;
                }
                if (yVelocity <= -(fallDamageVelocity)) {
                    takingFallDamage = true;
                    fallingTime += dt;
                }
            }
        protected:
            float fallDamageVelocity;
            bool takingFallDamage;
            int fallDamage;
            float fallingTime;

            DamageableComponent* damageComponent = nullptr;
            PhysicsObject* physObj = nullptr;
        };
    }
} //Damage y speed is -24.0f 

// keep track of float of t wth dt then when v > fdv set track t = 0 then in update if v - cv > fdv and t < allowed time, then take damage