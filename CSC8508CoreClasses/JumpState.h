#pragma once
#include "IState.h"
#include "PhysicsComponent.h"
#include "PhysicsObject.h"

namespace NCL {
	namespace CSC8508 {
		class JumpState : public IState {
		public:
			JumpState(float jump) {
				jumpForce = jump;
				physComponent = nullptr;
			}
			void Update(float dt, GameObject& gameObject) override {
				if (physComponent == NULL) {
					physComponent = gameObject.TryGetComponent<PhysicsComponent>();
				}
				physComponent->GetPhysicsObject()->AddForce(Vector3(0.0f, jumpForce, 0.0f));
			}

		protected:
			float jumpForce = 0.0f;
			PhysicsComponent* physComponent;
		};

	}
}