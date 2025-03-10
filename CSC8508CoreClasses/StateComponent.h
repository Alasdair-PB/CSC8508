#pragma once
#include "GameObject.h"
//#include "PhysicsComponent.h"
#include "IStateTransition.h"
#include "IState.h"
#include "IComponent.h"

namespace NCL {
    namespace CSC8508 {
        class IState;
        class IStateTransition;
        typedef std::multimap<IState*, IStateTransition*> ITransitionContainer;
        typedef ITransitionContainer::iterator ITransitionIterator;

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
            ITransitionContainer allTransitions;

            float counter;
        };
    }
}