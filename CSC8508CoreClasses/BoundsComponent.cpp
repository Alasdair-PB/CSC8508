//
// Contributors: Alasdair, Alfie
//

#include "BoundsComponent.h"

#include "Axis.h"
#include "CollisionDetection.h"

using namespace NCL::CSC8508;

BoundsComponent::BoundsComponent(GameObject& gameObject, CollisionVolume* collisionVolume, PhysicsComponent* physicsComponent) : IComponent(gameObject) {
	this->boundingVolume = collisionVolume;
	this->physicsComponent = physicsComponent;
	vector<Layers::LayerID> ignoreLayers = vector<Layers::LayerID>();
}

BoundsComponent::~BoundsComponent() { delete boundingVolume; }

bool BoundsComponent::GetBroadphaseAABB(Vector3& outSize) {
	if (!boundingVolume) {
		return false;
	}
	if (broadphaseAABB.IsEmpty()) UpdateBroadphaseAABB();
	outSize = broadphaseAABB;
	return true;
}

Vector3 GetOBBBroadphaseAABB(Quaternion const& orientation, Vector3 const& halfDimensions) {
	auto max = Vector3();

	// Get all world-orientated vertices (not repositioned)
	Vector3 array[8];
	for (int i = 0; i < 8; i++) {
		array[i] = orientation * (halfDimensions * Vector3(
			i & 1 ? 1 : -1,
			i & 2 ? 1 : -1,
			i & 4 ? 1 : -1
			));
	}

	// Check for max bounds
	for (Vector3 c : array) {
		for (Axis a = x; a <= z; a++) {
			if (fabs(c[a]) > max[a]) max[a] = c[a];
		}
	}
	return max;
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
	else if (static_cast<int>(boundingVolume->type) & static_cast<int>(VolumeType::Capsule)) {
		auto const vol = dynamic_cast<CapsuleVolume&>(*boundingVolume);
		float const r = vol.GetRadius();
		broadphaseAABB = Vector3(r, r + vol.GetHalfHeight(), r);
	}
	else if (static_cast<int>(boundingVolume->type) == static_cast<int>(VolumeType::OBB)) {
		broadphaseAABB = GetOBBBroadphaseAABB(GetGameObject().GetTransform().GetOrientation(), ((OBBVolume&)*boundingVolume).GetHalfDimensions());
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

	if (loadedSaveData.hasPhysics)
		physicsComponent = GetGameObject().TryGetComponent<PhysicsComponent>();
}
