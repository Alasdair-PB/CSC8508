#include "PhysicsObject.h"
#include "PhysicsSystem.h"
#include "Transform.h"
using namespace NCL;
using namespace CSC8508;

PhysicsObject::PhysicsObject(Transform* parentTransform)	{
	transform	= parentTransform;
	inverseMass = 1.0f;
	friction	= 0.8f;
}

PhysicsObject::~PhysicsObject()	{

}

void PhysicsObject::ApplyAngularImpulse(const Vector3& force) {
	angularVelocity += inverseInteriaTensor * force;
}

void PhysicsObject::ApplyLinearImpulse(const Vector3& force) {
	linearVelocity += force * inverseMass;
}

void PhysicsObject::AddForce(const Vector3& addedForce) {
	force += addedForce;
}

void PhysicsObject::AddForceAtPosition(const Vector3& addedForce, const Vector3& position) {
	Vector3 localPos = position - transform->GetPosition();

	force  += addedForce;
	torque += Vector::Cross(localPos, addedForce);
}

void PhysicsObject::AddTorque(const Vector3& addedTorque) {
	torque += addedTorque;
}

float PhysicsObject::GetFriction() { return friction; }

float PhysicsObject::GetCRestitution() { return cRestitution; }

void PhysicsObject::ClearForces() {
	force				= Vector3();
	torque				= Vector3();
}

void  PhysicsObject::RotateTowardsVelocity(float offset) {
	auto rotDir = Vector::Normalise(this->GetLinearVelocity());
	float angle = -std::atan2(rotDir.z, rotDir.x) * (180.0f / PI);
	angle += offset;
	Quaternion rot = Quaternion::AxisAngleToQuaterion(Vector3(0, 1, 0), angle);
	transform->SetOrientation(rot);
}

void PhysicsObject::InitCubeInertia() {
	Vector3 dimensions	= transform->GetScale();

	Vector3 fullWidth = dimensions * 2.0f;

	Vector3 dimsSqr		= fullWidth * fullWidth;

	inverseInertia.x = (12.0f * inverseMass) / (dimsSqr.y + dimsSqr.z);
	inverseInertia.y = (12.0f * inverseMass) / (dimsSqr.x + dimsSqr.z);
	inverseInertia.z = (12.0f * inverseMass) / (dimsSqr.x + dimsSqr.y);
}

void PhysicsObject::InitSphereInertia() {

	float radius	= Vector::GetMaxElement(transform->GetScale());
	float i			= 2.5f * inverseMass / (radius*radius);

	inverseInertia	= Vector3(i, i, i);
}

void PhysicsObject::UpdateInertiaTensor() {
	Quaternion q = transform->GetOrientation();

	Matrix3 invOrientation = Quaternion::RotationMatrix<Matrix3>(q.Conjugate());
	Matrix3 orientation = Quaternion::RotationMatrix<Matrix3>(q);

	inverseInteriaTensor = orientation * Matrix::Scale3x3(inverseInertia) *invOrientation;
}

