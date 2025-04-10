//
// Contributors: Alasdair
//

#include "BoundsComponent.h"

#include "Axis.h"
#include "CollisionDetection.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "INetworkDeltaComponent.h"
#include "Debug.h"

using namespace NCL::Maths;
using namespace NCL::CSC8508;

BoundsComponent::BoundsComponent(GameObject& gameObject, CollisionVolume* collisionVolume, PhysicsComponent* physicsComponent) : IComponent(gameObject) {
	this->boundingVolume = collisionVolume;
	this->physicsComponent = physicsComponent;
	vector<Layers::LayerID> ignoreLayers = vector<Layers::LayerID>();
#if EDITOR
	SetEditorData();
#endif
}

BoundsComponent::~BoundsComponent() { 
	delete boundingVolume; 
#if EDITOR
	delete expectingBoundsSize;
	delete isTrigger;
#endif
}

#if EDITOR

void BoundsComponent::SetBoundsInspectorScale(Vector3 scale){
	Vector3 objScale = GetGameObject().GetTransform().GetScale();
	scale =  (scale * 2.0f) / GetGameObject().GetTransform().GetScale();
	(*expectingBoundsSize) = scale;
}

Vector3 BoundsComponent::GetBoundsWorldScale() {
	return ((*expectingBoundsSize) * GetGameObject().GetTransform().GetScale())/2.0f;
}

void BoundsComponent::SetEditorData() {
	if (boundingVolume) {
		Vector3 scale = Vector3();
		(*isTrigger) = boundingVolume->isTrigger;
		switch (boundingVolume->type) {
			case VolumeType::AABB: {
				expectingVolumeType = 0;
				scale = dynamic_cast<AABBVolume*>(boundingVolume)->GetHalfDimensions();
				break;
			}
			case VolumeType::OBB: {
				expectingVolumeType = 1;
				scale = dynamic_cast<OBBVolume*>(boundingVolume)->GetHalfDimensions();
				break;
			}
			case VolumeType::Sphere: {
				expectingVolumeType = 2;
				scale.x = dynamic_cast<SphereVolume*>(boundingVolume)->GetRadius();
				break;
			}
			case VolumeType::Capsule: {
				expectingVolumeType = 3;
				scale.x = dynamic_cast<CapsuleVolume*>(boundingVolume)->GetRadius();
				scale.y = dynamic_cast<CapsuleVolume*>(boundingVolume)->GetHalfHeight();
				break;
			}
			case VolumeType::Mesh: {
				expectingVolumeType = 4;
				scale = Vector3();
				return;
			}
			case VolumeType::Compound: {
				expectingVolumeType = 5;
				scale = Vector3();
				return;
			}
			case VolumeType::Invalid: {
				expectingVolumeType = 6;
				scale = Vector3();
				return;
			}
			default: {
				expectingVolumeType = 0;
				scale = Vector3();
				return;
			}
		}
		SetBoundsInspectorScale(scale);
	}
}
#endif

bool BoundsComponent::GetBroadphaseAABB(Vector3& outSize) {
	if (!boundingVolume) return false;
	if (broadphaseAABB.IsEmpty()) UpdateBroadphaseAABB();
	outSize = broadphaseAABB;
	return true;
}

Vector3 GetOBBBroadphaseAABB(Quaternion const& orientation, Vector3 const& halfDimensions) {
	auto max = Vector3();
	Vector3 array[8];
	for (int i = 0; i < 8; i++) {
		array[i] = orientation * (halfDimensions * Vector3(
			i & 1 ? 1 : -1,
			i & 2 ? 1 : -1,
			i & 4 ? 1 : -1
			));
	}
	for (Vector3 c : array) {
		for (Axis a = x; a <= z; a++)
			if (fabs(c[a]) > max[a]) max[a] = c[a];
	}
	return max;
}

void BoundsComponent::UpdateBroadphaseAABB() {
	if (!boundingVolume) {return;}
	if (static_cast<int>(boundingVolume->type) & static_cast<int>(VolumeType::AABB))
		broadphaseAABB = ((AABBVolume&)*boundingVolume).GetHalfDimensions();
	else if (static_cast<int>(boundingVolume->type) & static_cast<int>(VolumeType::Sphere)) {
		float r = ((SphereVolume&)*boundingVolume).GetRadius();
		broadphaseAABB = Vector3(r, r, r);
	}
	else if (static_cast<int>(boundingVolume->type) & static_cast<int>(VolumeType::Capsule)) {
		auto const vol = dynamic_cast<CapsuleVolume&>(*boundingVolume);
		float const r = vol.GetRadius();
		broadphaseAABB = Vector3(r, r + vol.GetHalfHeight(), r);
	}
	else if (static_cast<int>(boundingVolume->type) & static_cast<int>(VolumeType::OBB))
		broadphaseAABB = GetOBBBroadphaseAABB(GetGameObject().GetTransform().GetOrientation(), ((OBBVolume&)*boundingVolume).GetHalfDimensions());
}

struct BoundsComponent::BoundsComponentDataStruct : public ISerializedData {
	BoundsComponentDataStruct() : enabled(true), isTrigger(false), hasPhysics(false), volumeType(VolumeType::AABB), boundsSize(Vector3(1, 1, 1)) {}
	BoundsComponentDataStruct(bool enabled, bool isTrigger, bool hasPhysics, VolumeType volumeType, Vector3 boundsSize, vector<Layers::LayerID> ignoreLayers) :
		enabled(enabled), isTrigger(isTrigger), hasPhysics(hasPhysics), volumeType(volumeType), boundsSize(boundsSize), ignoreLayers(ignoreLayers){}
	bool enabled;
	bool isTrigger;
	bool hasPhysics;
	VolumeType volumeType;
	Vector3 boundsSize;
	vector<Layers::LayerID> ignoreLayers;

	static auto GetSerializedFields() {
		return std::make_tuple(
			SERIALIZED_FIELD(BoundsComponentDataStruct, enabled),
			SERIALIZED_FIELD(BoundsComponentDataStruct, isTrigger),
			SERIALIZED_FIELD(BoundsComponentDataStruct, hasPhysics),
			SERIALIZED_FIELD(BoundsComponentDataStruct, volumeType),
			SERIALIZED_FIELD(BoundsComponentDataStruct, boundsSize),
			SERIALIZED_FIELD(BoundsComponentDataStruct, ignoreLayers)
		);
	}
};

void BoundsComponent::CopyComponent(GameObject* gameObject) {
	CollisionVolume* volume = nullptr;
	PhysicsComponent* physics = nullptr;
	if (boundingVolume)
		volume = CopyVolume(boundingVolume->isTrigger, boundingVolume->type, GetBoundsScale());
	if (physicsComponent)
		physics = gameObject->TryGetComponent<PhysicsComponent>();

	BoundsComponent* component = gameObject->AddComponent<BoundsComponent>(volume, physics);
	component->SetEnabled(IsEnabled());

#if EDITOR
	SetEditorData();
#endif
}

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
		case VolumeType::Mesh: {return Vector3(1, 1, 1);}
		case VolumeType::Compound: {return Vector3(1, 1, 1);}
		case VolumeType::Invalid: {return Vector3(1, 1, 1);}
		default: {return Vector3(1, 1, 1);}
	}
}

NCL::CollisionVolume* BoundsComponent::CopyVolume(bool isTrigger, VolumeType volumeType, Vector3 boundsSize) {
	NCL::CollisionVolume* volume = nullptr;
	switch (volumeType) {
		case VolumeType::AABB: {
			volume = new AABBVolume(boundsSize);
		break;}
		case VolumeType::OBB: {
			volume = new OBBVolume(boundsSize);
		break;}
		case VolumeType::Sphere: {
			volume = new SphereVolume(boundsSize.x);
		break;}
		case VolumeType::Capsule: {
			volume = new CapsuleVolume(boundsSize.y, boundsSize.x);
		break;}
		case VolumeType::Mesh: {break;}
		case VolumeType::Compound: {break;}
		case VolumeType::Invalid: {break;}
		default: { break;}
	}
	if (volume) volume->isTrigger = isTrigger;
	return volume;
}

void BoundsComponent::LoadVolume(bool isTrigger, VolumeType volumeType, Vector3 boundsSize, CollisionVolume* volume) {
	switch (volumeType) {
		case VolumeType::AABB: {
			boundingVolume = new AABBVolume(boundsSize);
		break;}
		case VolumeType::OBB: {
			boundingVolume = new OBBVolume(boundsSize);
		break;}
		case VolumeType::Sphere: {
			boundingVolume = new SphereVolume(boundsSize.x);
		break;}
		case VolumeType::Capsule: {
			boundingVolume = new CapsuleVolume(boundsSize.y, boundsSize.x);
		break;}
		case VolumeType::Mesh: { return; }
		case VolumeType::Compound: { return; }
		case VolumeType::Invalid: { return; }
		default: { return; }
	}
	boundingVolume->isTrigger = isTrigger;
}

auto BoundsComponent::GetDerivedSerializedFields() const {
	return BoundsComponentDataStruct::GetSerializedFields();
}

size_t BoundsComponent::Save(std::string assetPath, size_t* allocationStart)
{
	BoundsComponentDataStruct saveInfo;
	saveInfo = boundingVolume == nullptr ? BoundsComponentDataStruct() :
	BoundsComponentDataStruct(IsEnabled(), boundingVolume->isTrigger, (physicsComponent != nullptr), boundingVolume->type, GetBoundsScale(), ignoreLayers);
	SaveManager::GameData saveData = ISerializedData::CreateGameData<BoundsComponentDataStruct>(saveInfo);
	return SaveManager::SaveGameData(assetPath, saveData, allocationStart, false);
}

void BoundsComponent::Load(std::string assetPath, size_t allocationStart) {
	BoundsComponentDataStruct loadedSaveData = ISerializedData::LoadISerializable<BoundsComponentDataStruct>(assetPath, allocationStart);
	LoadVolume(loadedSaveData.isTrigger, loadedSaveData.volumeType, loadedSaveData.boundsSize, boundingVolume);
	SetEnabled(loadedSaveData.enabled);
	ignoreLayers = loadedSaveData.ignoreLayers;
	if (loadedSaveData.hasPhysics)
		physicsComponent = GetGameObject().TryGetComponent<PhysicsComponent>();
	#if EDITOR
		SetEditorData();
	#endif
}

void BoundsComponent::PushIComponentElementsInspector(UIElementsGroup& elementsGroup, float scale) {
#if EDITOR
	elementsGroup.PushStaticTextElement("Bounds Component");
	for (Layers::LayerID& layer : ignoreLayers) {
		std::vector<std::pair<int*, std::string>> enumTagOptions = {
			{reinterpret_cast<int*>(&layer), "Default"},
			{reinterpret_cast<int*>(&layer), "Ignore_RayCast"},
			{reinterpret_cast<int*>(&layer), "UI"},
			{reinterpret_cast<int*>(&layer), "Player"},
			{reinterpret_cast<int*>(&layer), "Enemy"},
			{reinterpret_cast<int*>(&layer), "Ignore_Collisions"}
		};
		elementsGroup.PushEnumElement("Ignored Layer", enumTagOptions);
	}
	elementsGroup.PushStatelessButtonElement(ImVec2(scale, scale/2), "Add Layer",
		[this]() { AddToIgnoredLayers(Layers::Default);});
	elementsGroup.PushStatelessButtonElement(ImVec2(scale, scale/2), "Remove Layer",
		[this]() { if (!ignoreLayers.empty()) ignoreLayers.pop_back();});

	if (!physicsComponent) {
		elementsGroup.PushStatelessButtonElement(ImVec2(scale, scale/2), "Link PhysicsComponent",
			[this]() {SetPhysicsComponent(GetGameObject().TryGetComponent<PhysicsComponent>()); });
	}
	else {
		elementsGroup.PushStatelessButtonElement(ImVec2(scale, scale/2), "UnLink PhysicsComponent",
			[this]() {SetPhysicsComponent(nullptr); });
	}
	if (boundingVolume) {
		std::vector<std::pair<int*, std::string>> enumOptions = {
			{&expectingVolumeType, "AABB"},
			{&expectingVolumeType, "OBB"},
			{&expectingVolumeType, "Sphere"},
			{&expectingVolumeType, "Mesh"},
			{&expectingVolumeType, "Capsule"},
			{&expectingVolumeType, "Compound"},
			{&expectingVolumeType, "Invalid"}
		};

		elementsGroup.PushEnumElement("VolumeType", enumOptions);
		elementsGroup.PushToggle("Is Trigger", isTrigger, scale);

		bool boundsMatch = true;
		VolumeType type = enumVolumeCast[expectingVolumeType];
		Vector3 convertedScale = GetBoundsWorldScale();

		switch (boundingVolume->type) {
			case VolumeType::AABB: {
				elementsGroup.PushVectorElement(expectingBoundsSize, scale, "Dimensions");
				Vector3 halfDim = dynamic_cast<AABBVolume*>(boundingVolume)->GetHalfDimensions();

				if (convertedScale.x != halfDim.x || convertedScale.y != halfDim.y || convertedScale.z != halfDim.z)
					boundsMatch = false;
			break; }
			case VolumeType::OBB: {
				elementsGroup.PushVectorElement(expectingBoundsSize, scale, "Dimensions");
				Vector3 halfDim = dynamic_cast<OBBVolume*>(boundingVolume)->GetHalfDimensions();

				if (convertedScale.x != halfDim.x || convertedScale.y != halfDim.y || convertedScale.z != halfDim.z)
					boundsMatch = false;
			break; }
			case VolumeType::Sphere: {
				elementsGroup.PushFloatElement(&expectingBoundsSize->x, scale, "Radius:");

				if (convertedScale.x != dynamic_cast<SphereVolume*>(boundingVolume)->GetRadius())
					boundsMatch = false;
			break; }
			case VolumeType::Capsule: {
				elementsGroup.PushFloatElement(&expectingBoundsSize->x, scale, "HalfHeight:");
				elementsGroup.PushFloatElement(&expectingBoundsSize->y, scale, "Radius:");

				if (convertedScale.x != dynamic_cast<CapsuleVolume*>(boundingVolume)->GetRadius()||
					convertedScale.y != dynamic_cast<CapsuleVolume*>(boundingVolume)->GetHalfHeight())
					boundsMatch = false;
			break; }
			case VolumeType::Mesh: {break;}
			case VolumeType::Compound: {break;}
			case VolumeType::Invalid: { break;}
			default: { break; }
		}
		if (type != boundingVolume->type || ((*isTrigger) != boundingVolume->isTrigger) || !boundsMatch)
			LoadVolume(*isTrigger, type, convertedScale, boundingVolume);
		elementsGroup.PushStatelessButtonElement(ImVec2(scale, scale/2), "Remove Bounding Volume",
			[this]() {SetBoundingVolume(nullptr); });
	}
	else {
		elementsGroup.PushStatelessButtonElement(ImVec2(scale, scale/2), "Add Bounding Volume",
			[this]() {LoadVolume(*isTrigger, VolumeType::AABB, *expectingBoundsSize, boundingVolume); });
	}
#endif
}

