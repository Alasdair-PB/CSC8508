#pragma once
#include "GameObject.h"
#include "PhysicsComponent.h"


namespace NCL {
    namespace CSC8508 {
        class IState;
        class IStateTransition;
        typedef std::multimap<IState*, IStateTransition*> TransitionContainer;
        typedef TransitionContainer::iterator TransitionIterator;

        class StateComponent : public IComponent {
        public:
            StateComponent(GameObject& gameObject);
            ~StateComponent();

            void AddState(IState* s);
            void AddTransition(IStateTransition* t);

            virtual void Update(float dt, GameObject& gameObject);


        protected:
            IState* activeState;
            
            std::vector<IState*> allStates;
            TransitionContainer allTransitions;

            float counter;
        };
    }
}