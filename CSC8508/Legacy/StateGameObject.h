#pragma once
#include "GameObject.h"
#include "PhysicsComponent.h"


namespace NCL {
    namespace CSC8508 {
        class StateMachine;
        class StateGameObject : public GameObject  {
        public:
            StateGameObject();
            ~StateGameObject();

            virtual void Update(float dt);

            void OnAwake() override
            {
                physics = this->TryGetComponent<PhysicsComponent>();
            }

        protected:
            void MoveLeft(float dt);
            void MoveRight(float dt);

            StateMachine* stateMachine;
            PhysicsComponent* physics;
            float counter;
        };
    }
}
