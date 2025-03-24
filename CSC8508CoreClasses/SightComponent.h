#pragma once
#include "IComponent.h"
#include "GameWorld.h"

namespace NCL {
    namespace CSC8508 {

        class SightComponent : public IComponent {
        public:
            SightComponent(GameObject& g):worldInstance(GameWorld::Instance()), IComponent(g) {

            }
            ~SightComponent();

            template<typename t>GameObject* GetClosestVisibleTarget(float maxRange) {

            }

            void OnAwake() override {

            }

        protected:
            GameWorld& worldInstance;
        };
    }
}