#include "StateComponent.h"
#include "IState.h"
#include "IStateTransition.h"

using namespace NCL::CSC8508;

StateComponent::StateComponent(GameObject& gameObject) : IComponent(gameObject){
	activeState = nullptr;
	counter = 0.0f;
}

StateComponent::~StateComponent() {
	for (auto& i : allStates) {
		delete i;
	}
	for (auto& i : allTransitions) {
		delete i.second;
	}
}

void StateComponent::AddState(IState* s) {
	allStates.emplace_back(s);
	if (activeState == nullptr) {
		activeState = s;
	}
}

void StateComponent::AddTransition(IStateTransition* t) {
	allTransitions.insert(std::make_pair(t->GetSourceState(), t));
}


void StateComponent::Update(float dt, GameObject& gamneObject) {
	if (activeState) {
		activeState->Update(dt, gamneObject);

		std::pair<TransitionIterator, TransitionIterator> range = allTransitions.equal_range(activeState);

		for (auto& i = range.first; i != range.second; ++i) {
			if (i->second->CanTransition()) {
				IState* newState = i->second->GetDestinationState();
				activeState = newState;
			}
		}
	}
}