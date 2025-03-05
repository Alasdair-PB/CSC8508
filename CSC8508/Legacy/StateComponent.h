#pragma once
#include "GameObject.h"
#include "PhysicsComponent.h"


namespace NCL {
    namespace CSC8508 {
        class StateMachine;
        class StateComponent : public IComponent  {
        public:
            StateComponent(GameObject& gameObject);
            ~StateComponent();

            virtual void Update(float dt);

            void OnAwake() override
            {
                physics = GetGameObject().TryGetComponent<PhysicsComponent>();
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
