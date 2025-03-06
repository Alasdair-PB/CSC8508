#pragma once
#include "IState.h"

namespace NCL {
	namespace CSC8508 {
		class WaitState : public IState {
		public:
			WaitState(float waitLength) {
				waitDuration = waitLength;
				waitTime = 0.0f;
			}
			void Update(float dt, GameObject& gameObject) override{
				waitTime += dt;
			}
			bool IsComplete(float dt, GameObject& gameObject) override {
				return waitDuration <= waitTime ? true : false;
			}

		protected:
			float waitTime = 0.0f;
			float waitDuration = 0.0f;
		};
	
	}
}