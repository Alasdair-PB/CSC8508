//
// Contributors: Alasdair
//

#include "BoundsComponent.h"
#include "CollisionDetection.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "NetworkObject.h"

using namespace NCL::CSC8508;

BoundsComponent::BoundsComponent(GameObject& gameObject, CollisionVolume* collisionVolume, PhysicsComponent* physicsComponent) : IComponent(gameObject) {
	this->boundingVolume = collisionVolume;
	this->physicsComponent = physicsComponent;
	vector<Layers::LayerID> ignoreLayers = vector<Layers::LayerID>();
}

BoundsComponent::~BoundsComponent() {
	delete boundingVolume;
}

bool BoundsComponent::GetBroadphaseAABB(Vector3& outSize){
	if (!boundingVolume) {
		return false;
	}
	if (broadphaseAABB.IsEmpty()) UpdateBroadphaseAABB();
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
		std::cout << "Tried to get broadphase of OBB but it doesn't exist yet! Soz!\n";
		/*Matrix3 mat = Quaternion::RotationMatrix<Matrix3>(transform.GetOrientation());
		mat = Matrix::Absolute(mat);
		Vector3 halfSizes = ((OBBVolume&)*boundingVolume).GetHalfDimensions();
		broadphaseAABB = mat * halfSizes;*/
	}
}