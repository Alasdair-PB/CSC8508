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

BoundsComponent::~BoundsComponent() { delete boundingVolume; }

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

struct BoundsComponent::BoundsComponentDataStruct : public ISerializedData {
	BoundsComponentDataStruct() : enabled(true), isTrigger(false), hasPhysics(false), volumeType(VolumeType::AABB), boundsSize(Vector3(1, 1, 1)) {}
	BoundsComponentDataStruct(bool enabled, bool isTrigger, bool hasPhysics, VolumeType volumeType, Vector3 boundsSize) :
		enabled(enabled), isTrigger(isTrigger), hasPhysics(hasPhysics), volumeType(volumeType), boundsSize(boundsSize) {}
	bool enabled;
	bool isTrigger;
	bool hasPhysics;
	VolumeType volumeType;
	Vector3 boundsSize;

	static auto GetSerializedFields() {
		return std::make_tuple(
			SERIALIZED_FIELD(BoundsComponentDataStruct, enabled),
			SERIALIZED_FIELD(BoundsComponentDataStruct, isTrigger),
			SERIALIZED_FIELD(BoundsComponentDataStruct, hasPhysics),
			SERIALIZED_FIELD(BoundsComponentDataStruct, volumeType),
			SERIALIZED_FIELD(BoundsComponentDataStruct, boundsSize)
		);
	}
};


Vector3 BoundsComponent::GetBoundsScale() {
	switch (boundingVolume->type) {
	case VolumeType::AABB: {
		CapsuleVolume* capsule = dynamic_cast<CapsuleVolume*>(boundingVolume);
		return capsule == nullptr ? Vector3(1, 1, 1) : Vector3(capsule->GetRadius(), capsule->GetHalfHeight(), 0);
	}
	case VolumeType::OBB: {
		OBBVolume* obb = dynamic_cast<OBBVolume*>(boundingVolume);
		return obb == nullptr ? Vector3(1,1,1) : obb->GetHalfDimensions();
	}
	case VolumeType::Sphere: {
		SphereVolume* sphere = dynamic_cast<SphereVolume*>(boundingVolume);
		return sphere == nullptr ? Vector3(1, 1, 1) : Vector3(sphere->GetRadius(), 0, 0);
	}
	case VolumeType::Capsule: {
		CapsuleVolume* capsule = dynamic_cast<CapsuleVolume*>(boundingVolume);
		return capsule == nullptr ? Vector3(1, 1, 1) : Vector3(capsule->GetRadius(), capsule->GetHalfHeight(), 0);
	}
	case VolumeType::Mesh: {
		return Vector3(1, 1, 1);
	}
	case VolumeType::Compound: {
		return Vector3(1, 1, 1);
	}
	case VolumeType::Invalid: {
		return Vector3(1, 1, 1);
	}
	default: {
		return Vector3(1, 1, 1);
	}
	}
}

void BoundsComponent::LoadVolume(bool isTrigger, VolumeType volumeType, Vector3 boundsSize) {		
	switch (volumeType) {
	case VolumeType::AABB: {
		boundingVolume = new AABBVolume(boundsSize);
		break;
	}
	case VolumeType::OBB: {
		boundingVolume = new OBBVolume(boundsSize);
		break;
	}
	case VolumeType::Sphere: {
		boundingVolume = new SphereVolume(boundsSize.x);
			break;
	}
	case VolumeType::Capsule: {
		boundingVolume = new CapsuleVolume(boundsSize.y, boundsSize.x);
		break;
	}
	case VolumeType::Mesh: {
		return;
	}
	case VolumeType::Compound: {
		return;
	}
	case VolumeType::Invalid: {
		return;
	}
	default: {
		return;
	}
	}
	boundingVolume->isTrigger = isTrigger;
}

size_t BoundsComponent::Save(std::string assetPath, size_t* allocationStart)
{
	BoundsComponentDataStruct saveInfo;
	saveInfo = boundingVolume == nullptr ? BoundsComponentDataStruct() :
		BoundsComponentDataStruct(IsEnabled(), boundingVolume->isTrigger, (physicsComponent != nullptr), boundingVolume->type, GetBoundsScale());
	SaveManager::GameData saveData = ISerializedData::CreateGameData<BoundsComponentDataStruct>(saveInfo);
	return SaveManager::SaveGameData(assetPath, saveData, allocationStart, false);
}

void BoundsComponent::Load(std::string assetPath, size_t allocationStart) {
	BoundsComponentDataStruct loadedSaveData = ISerializedData::LoadISerializable<BoundsComponentDataStruct>(assetPath, allocationStart);
	LoadVolume(loadedSaveData.isTrigger, loadedSaveData.volumeType, loadedSaveData.boundsSize);
	SetEnabled(loadedSaveData.enabled);

	if (loadedSaveData.hasPhysics) {
		physicsComponent = GetGameObject().TryGetComponent<PhysicsComponent>();
	}
}
