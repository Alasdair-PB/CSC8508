#pragma once
#include "Constraint.h"

namespace NCL {
	namespace CSC8508 {
		class PhysicsComponent;

		class PositionConstraint : public Constraint	{
		public:
			PositionConstraint(PhysicsComponent* a, PhysicsComponent* b, float d);
			~PositionConstraint();

			void UpdateConstraint(float dt) override;

		protected:
			PhysicsComponent* objectA;
			PhysicsComponent* objectB;

			float distance;
		};
	}
}