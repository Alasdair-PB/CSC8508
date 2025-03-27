#pragma once
#include "IComponent.h"
#include "../CSC8508/DamageableComponent.h"
#include "PhysicsComponent.h"
#include "PhysicsObject.h"

namespace NCL {
    namespace CSC8508 {

        class FallDamageComponent : public IComponent {
        public:
            FallDamageComponent(GameObject& gameObject, float Y, int D):IComponent(gameObject) {
                fallDamageVelocity = Y;
                takingFallDamage = false;
                fallDamage = D;
            }
            ~FallDamageComponent();

            void OnAwake() override {
                damageComponent = GetGameObject().TryGetComponent<DamageableComponent>();
                physObj = GetGameObject().TryGetComponent<PhysicsComponent>()->GetPhysicsObject();
            }

            void Update(float dt) override {
                float yVelocity = physObj->GetLinearVelocity().y;
                if (abs(yVelocity) < FLT_EPSILON && takingFallDamage == true) {
                    //flag take damage
                }
                if (yVelocity >= fallDamageVelocity) {
                    takingFallDamage = true;
                }
            }
        protected:
            float fallDamageVelocity;
            bool takingFallDamage;
            int fallDamage;

            DamageableComponent* damageComponent;
            PhysicsObject* physObj;
        };
    }
} //Damage y speed is -24.0f 

// keep track of float of t wth dt then when v > fdv set track t = 0 then in update if v - cv > fdv + t < allowed time, then take damage