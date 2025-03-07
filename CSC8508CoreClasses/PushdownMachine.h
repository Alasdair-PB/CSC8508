#pragma once
#include "PushdownState.h"

namespace NCL {
	namespace CSC8508 {
		class PushdownState;

		class PushdownMachine
		{
		public:
			PushdownMachine(PushdownState* initialState);
			~PushdownMachine();

			void PushStateToStack(PushdownState* newState);
			PushdownState* GetActiveState();

			bool Update(float dt);
			bool ModifyStack(PushdownState* newState, PushdownState::PushdownResult result);

		protected:
			PushdownState* activeState;
			PushdownState* initialState;

			std::stack<PushdownState*> stateStack;
		};
	}
}

