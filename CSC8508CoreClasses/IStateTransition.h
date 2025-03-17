#pragma once

namespace NCL {
	namespace CSC8508 {
		class IState;
		typedef std::function<bool()> IStateTransitionFunction;
		class IStateTransition {
		public:
			IStateTransition(IState* source, IState* dest, IStateTransitionFunction f) {
				sourceState = source;
				destinationState = dest;
				function = f;
			}

			bool CanTransition() const {
				return function();
			}

			IState* GetDestinationState()  const {
				return destinationState;
			}

			IState* GetSourceState() const {
				return sourceState;
			}

		protected:
			IState* sourceState;
			IState* destinationState;
			IStateTransitionFunction function;
		};
	}
}