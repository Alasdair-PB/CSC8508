#include "BoundsComponent.h"
#include "CollisionDetection.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "NetworkObject.h"

using namespace NCL::CSC8508;

BoundsComponent::BoundsComponent(GameObject& gameObject, CollisionVolume* collisionVolume) : IComponent(gameObject) {
	boundingVolume = collisionVolume;
	vector<Layers::LayerID> ignoreLayers = vector<Layers::LayerID>();
}

BoundsComponent::~BoundsComponent() {
	delete boundingVolume;
}

void BoundsComponent::Update(float deltaTime) {
	if (boundingVolume) {
	}
}

void BoundsComponent::LateUpdate(float deltaTime) {
	if (boundingVolume) {
	}
}

void BoundsComponent::OnEnable() {
}

void BoundsComponent::OnDisable() {
}

bool BoundsComponent::GetBroadphaseAABB(Vector3& outSize) const {
	if (!boundingVolume) {
		return false;
	}
	outSize = broadphaseAABB;
	return true;
}

void BoundsComponent::UpdateBroadphaseAABB() {
	if (!boundingVolume) {
		return;
	}
	if (static_cast<int>(boundingVolume->type) & static_cast<int>(VolumeType::AABB)) {
		broadphaseAABB = ((AABBVolume&)*boundingVolume).GetHalfDimensions();
	}
	else if (static_cast<int>(boundingVolume->type) & static_cast<int>(VolumeType::Sphere)) {
		float r = ((SphereVolume&)*boundingVolume).GetRadius();
		broadphaseAABB = Vector3(r, r, r);
	}
	else if (static_cast<int>(boundingVolume->type) == static_cast<int>(VolumeType::OBB)) {
		/*Matrix3 mat = Quaternion::RotationMatrix<Matrix3>(transform.GetOrientation());
		mat = Matrix::Absolute(mat);
		Vector3 halfSizes = ((OBBVolume&)*boundingVolume).GetHalfDimensions();
		broadphaseAABB = mat * halfSizes;*/
	}
}