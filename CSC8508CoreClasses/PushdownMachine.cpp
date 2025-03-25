#include "PushdownMachine.h"
#include "PushdownState.h"

using namespace NCL::CSC8508;


PushdownMachine::PushdownMachine(PushdownState* initialState){
	this->initialState = initialState;
	this->activeState = nullptr;
}

PushdownMachine::~PushdownMachine(){}

void PushdownMachine::PushStateToStack(PushdownState* newState) {ModifyStack(newState, PushdownState::Push);}

PushdownState* PushdownMachine::GetActiveState() {return activeState;}


bool  PushdownMachine::ModifyStack(PushdownState* newState, PushdownState::PushdownResult result) {

	if (!activeState) 
	{
		stateStack.push(initialState);
		activeState = initialState;
		activeState->OnAwake();
	}

	switch (result) {
		case PushdownState::Pop: 
		{
			activeState->OnSleep();
			delete activeState;
			stateStack.pop();

			if (stateStack.empty()) 
				return false;
			else {
				activeState = stateStack.top();
				activeState->OnAwake();
			}
		}break;
		case PushdownState::Push: {
			activeState->OnSleep();

			stateStack.push(newState);
			activeState = newState;
			activeState->OnAwake();
		}break;
	}
	return true;
}

bool PushdownMachine::Update(float dt) 
{
	if (activeState) {
		PushdownState* newState = nullptr;
		PushdownState::PushdownResult result = activeState->OnUpdate(dt, &newState);

		if (!ModifyStack(newState, result))
			return false;
	}
	else {
		stateStack.push(initialState);
		activeState = initialState;
		activeState->OnAwake();
	}
	return true;
}