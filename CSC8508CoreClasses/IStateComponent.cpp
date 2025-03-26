#include "IStateComponent.h"
using namespace NCL::CSC8508;

IStateComponent::IStateComponent(GameObject& gameObject) : IComponent(gameObject){
	activeState = nullptr;
	counter = 0.0f;
}

IStateComponent::~IStateComponent() {
	for (auto& i : allStates) {
		delete i;
	}
	for (auto& i : allTransitions) {
		delete i.second;
	}
}

void IStateComponent::AddState(IState* s) {
	allStates.emplace_back(s);
	if (activeState == nullptr) {
		activeState = s;
	}
}

void IStateComponent::AddTransition(IStateTransition* t) {
	allTransitions.insert(std::make_pair(t->GetSourceState(), t));
}


void IStateComponent::Update(float dt, GameObject& gamneObject) {
	if (activeState) {
		activeState->Update(dt, gamneObject);

		std::pair<ITransitionIterator, ITransitionIterator> range = allTransitions.equal_range(activeState);

		for (auto& i = range.first; i != range.second; ++i) {
			if (i->second->CanTransition()) {
				IState* newState = i->second->GetDestinationState();
				activeState = newState;
			}
		}
	}
}