#pragma once
#include "GameObject.h"
#include "PhysicsComponent.h"
#include "StateComponent.h"


namespace NCL {
    namespace CSC8508 {
        class IState;
        class IStateTransition;
        

        class JumpWaitStateComponent : public StateComponent {
        public:
            JumpWaitStateComponent(GameObject& gameObject) {
                activeState = nullptr;
            }
            ~JumpWaitStateComponent() {
                for (auto& i : allStates) {
                    delete i;
                }
                for (auto& i : allTransitions) {
                    delete i.second;
                }
            }

            void AddState(IState* s);
            void AddTransition(IStateTransition* t);

            virtual void Update(float dt, GameObject& gameObject);


        protected:

        };
    }
}