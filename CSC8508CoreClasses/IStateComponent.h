#pragma once
//#include "GameObject.h"
//#include "PhysicsComponent.h"

#include <map>

#include "IStateTransition.h"
#include "IState.h"
#include "IComponent.h"


namespace NCL {
    namespace CSC8508 {
        //class IState;
        //class IStateTransition;
        typedef std::multimap<IState*, IStateTransition*> ITransitionContainer;
        typedef ITransitionContainer::iterator ITransitionIterator;

        class IStateComponent : public IComponent {
        public:
            IStateComponent(GameObject& gameObject);
            ~IStateComponent();

            void AddState(IState* s);
            void AddTransition(IStateTransition* t);

            virtual void Update(float dt, GameObject& gameObject);


        protected:
            virtual void SetActiveState(IState* state) { activeState = state; }
            IState* activeState;
            
            std::vector<IState*> allStates;
            ITransitionContainer allTransitions;

            float counter;
        };
    }
}