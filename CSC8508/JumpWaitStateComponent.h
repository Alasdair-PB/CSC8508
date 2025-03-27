#pragma once
#include "GameObject.h"
#include "PhysicsComponent.h"
#include "IStateComponent.h"
#include "WaitState.h"
#include "JumpState.h"
#include "IStateTransition.h"


namespace NCL {
    namespace CSC8508 {
        class IState;
        class IStateTransition;
        

        class JumpWaitStateComponent : public IStateComponent {
        public:
            JumpWaitStateComponent(GameObject& gameObject) : IStateComponent(gameObject){
                activeState = nullptr;

                IState* stateA = new WaitState(10.0f);
                IState* stateB = new JumpState(15.0f);

                this->AddState(stateA);
                this->AddState(stateB);

                this->AddTransition(new IStateTransition(stateA, stateB, [&]()->bool {
                    return stateA->IsComplete(GetGameObject());
                    }));
                this->AddTransition(new IStateTransition(stateB, stateA, [&]()->bool {
                    return true;
                    }));

            }
            ~JumpWaitStateComponent() {
                for (auto& i : allStates) {
                    delete i;
                }
                for (auto& i : allTransitions) {
                    delete i.second;
                }
            }
        protected:

        };
    }
}