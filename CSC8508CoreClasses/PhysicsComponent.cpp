#include "PhysicsComponent.h"
#include "CollisionDetection.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "NetworkObject.h"

using namespace NCL::CSC8508;

PhysicsComponent::PhysicsComponent(GameObject* gameObject) : IComponent(gameObject) {
	boundingVolume = nullptr;
	physicsObject = nullptr;
	vector<PhysicsLayers::LayerID> ignoreLayers = vector<PhysicsLayers::LayerID>();
}

PhysicsComponent::~PhysicsComponent() {
	delete boundingVolume;
	delete physicsObject;
}

void PhysicsComponent::Update(float deltaTime) {
	if (physicsObject) {
	}
}

void PhysicsComponent::LateUpdate(float deltaTime) {
	if (physicsObject) {
	}
}

void PhysicsComponent::OnEnable() {
}

void PhysicsComponent::OnDisable() {
}

bool PhysicsComponent::GetBroadphaseAABB(Vector3& outSize) const {
	if (!boundingVolume) {
		return false;
	}
	outSize = broadphaseAABB;
	return true;
}

void PhysicsComponent::UpdateBroadphaseAABB() {
	if (!boundingVolume) {
		return;
	}
	if (boundingVolume->type == VolumeType::AABB) {
		broadphaseAABB = ((AABBVolume&)*boundingVolume).GetHalfDimensions();
	}
	else if (boundingVolume->type == VolumeType::Sphere) {
		float r = ((SphereVolume&)*boundingVolume).GetRadius();
		broadphaseAABB = Vector3(r, r, r);
	}
	else if (boundingVolume->type == VolumeType::OBB) {
		/*Matrix3 mat = Quaternion::RotationMatrix<Matrix3>(transform.GetOrientation());
		mat = Matrix::Absolute(mat);
		Vector3 halfSizes = ((OBBVolume&)*boundingVolume).GetHalfDimensions();
		broadphaseAABB = mat * halfSizes;*/
	}
}