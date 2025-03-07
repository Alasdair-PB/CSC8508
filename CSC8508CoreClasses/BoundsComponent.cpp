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
	BoundsComponentDataStruct() : enabled(true) {}
	BoundsComponentDataStruct(bool enabled) :
		enabled(enabled) {
	}
	bool enabled;
	// Create new collider from collider type enum
	// Bool try get physics component
	// Radius or vectors for bounds scale

	static auto GetSerializedFields() {
		return std::make_tuple(
			SERIALIZED_FIELD(BoundsComponentDataStruct, enabled)
		);
	}
};

size_t BoundsComponent::Save(std::string assetPath, size_t* allocationStart)
{
	BoundsComponentDataStruct saveInfo;
	saveInfo = BoundsComponentDataStruct(IsEnabled());
	SaveManager::GameData saveData = ISerializedData::CreateGameData<BoundsComponentDataStruct>(saveInfo);
	return SaveManager::SaveGameData(assetPath, saveData, allocationStart, false);
}

void BoundsComponent::Load(std::string assetPath, size_t allocationStart) {
	BoundsComponentDataStruct loadedSaveData = ISerializedData::LoadISerializable<BoundsComponentDataStruct>(assetPath, allocationStart);
	std::cout << loadedSaveData.enabled << ": Component is enabled" << std::endl;
}
