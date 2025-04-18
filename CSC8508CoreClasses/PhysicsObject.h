#pragma once
#include "Maths.h"

using namespace NCL::Maths;

namespace NCL {
	class CollisionVolume;
	
	namespace CSC8508 {
		class Transform;

		class PhysicsObject {
		public:
			PhysicsObject(Transform* parentTransform);
			~PhysicsObject();

			Vector3 GetLinearVelocity() const {
				return linearVelocity;
			}

			Vector3 GetAngularVelocity() const {
				return angularVelocity;
			}

			Vector3 GetTorque() const {
				return torque;
			}


			Vector3 GetForce() const {
				return force;
			}

			void SetInverseMass(float invMass) {
				inverseMass = invMass;
			}

			float GetInverseMass() const {
				return inverseMass;
			}

			float GetRestitution() { return cRestitution; }
			void SetRestitution(float newRestitution) { cRestitution = newRestitution; }

			void RotateTowardsVelocity(float offset = 90);

			void ApplyAngularImpulse(const Vector3& force);
			void ApplyLinearImpulse(const Vector3& force);

			void AddForce(const Vector3& force);

			void AddForceAtPosition(const Vector3& force, const Vector3& position);

			void AddTorque(const Vector3& torque);

			float GetFriction();
			void SetFriction(float friction) { this->friction = friction; }
			float GetCRestitution();

			void ClearForces();

			void SetLinearVelocity(const Vector3& v) {
				linearVelocity = v;
			}

			void SetAngularVelocity(const Vector3& v) {
				angularVelocity = v;
			}

			void InitCubeInertia();
			void InitSphereInertia();
			void IntegrateVelocity(float dt);
			void IntegrateAccel(float dt);

			void UpdateInertiaTensor();


			Matrix3 GetInertiaTensor() const {
				return inverseInteriaTensor;
			}

			float* GetInverseMassRef() {return &inverseMass;}
			float* GetFrictionRef() { return &friction; }
			float* GetcRestRef() { return &cRestitution; }

		protected:
			Transform* transform;

			float inverseMass;
			float friction;
			float cRestitution; 

			//linear stuff
			Vector3 linearVelocity;
			Vector3 force;
			
			//angular stuff
			Vector3 angularVelocity;
			Vector3 torque;
			Vector3 inverseInertia;
			Matrix3 inverseInteriaTensor;
		};
	}
}

