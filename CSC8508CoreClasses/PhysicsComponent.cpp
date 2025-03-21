#include "PhysicsComponent.h"
#include "CollisionDetection.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "NetworkObject.h"

using namespace NCL::CSC8508;

PhysicsComponent::PhysicsComponent(GameObject& gameObject) : IComponent(gameObject) {
	physicsObject = nullptr;
	vector<Layers::LayerID> ignoreLayers = vector<Layers::LayerID>();
}

PhysicsComponent::~PhysicsComponent() {
	delete physicsObject;
}

struct PhysicsComponent::PhysicsComponentDataStruct : public ISerializedData {
	PhysicsComponentDataStruct() : enabled(true), initType(None), inverseMass(0), friction(0), cRestitution(0){}
	PhysicsComponentDataStruct(bool enabled, InitType initType, float inverseMass, float friction, float cRestitution) :
		enabled(enabled), initType(initType), inverseMass(inverseMass), friction(friction), cRestitution(cRestitution) {}
	
	bool enabled;
	InitType initType;
	float inverseMass;
	float friction;
	float cRestitution;

	static auto GetSerializedFields() {
		return std::make_tuple(
			SERIALIZED_FIELD(PhysicsComponentDataStruct, enabled),
			SERIALIZED_FIELD(PhysicsComponentDataStruct, initType),
			SERIALIZED_FIELD(PhysicsComponentDataStruct, inverseMass),
			SERIALIZED_FIELD(PhysicsComponentDataStruct, friction),
			SERIALIZED_FIELD(PhysicsComponentDataStruct, cRestitution)
		);
	}
};

auto PhysicsComponent::GetSerializedFields() const {
	return PhysicsComponentDataStruct::GetSerializedFields();
}

size_t PhysicsComponent::Save(std::string assetPath, size_t* allocationStart)
{
	PhysicsComponentDataStruct saveInfo;
	saveInfo = physicsObject == nullptr ? PhysicsComponentDataStruct():
		PhysicsComponentDataStruct(IsEnabled(), initiType, physicsObject->GetInverseMass(), physicsObject->GetFriction(), physicsObject->GetCRestitution());
	SaveManager::GameData saveData = ISerializedData::CreateGameData<PhysicsComponentDataStruct>(saveInfo);
	return SaveManager::SaveGameData(assetPath, saveData, allocationStart, false);
}

void PhysicsComponent::Load(std::string assetPath, size_t allocationStart) {

	PhysicsComponentDataStruct loadedSaveData = ISerializedData::LoadISerializable<PhysicsComponentDataStruct>(assetPath, allocationStart);
	SetPhysicsObject(new PhysicsObject(&GetGameObject().GetTransform()));

	physicsObject->SetInverseMass(loadedSaveData.inverseMass);
	physicsObject->SetFriction(loadedSaveData.friction);
	physicsObject->SetRestitution(loadedSaveData.cRestitution);	
	
	switch (loadedSaveData.initType)
	{
		case(Sphere): {
			physicsObject->InitSphereInertia();
			break;
		}
		case(Cube): {
			physicsObject->InitCubeInertia();
			break;
		}
		default:
			break;
	}
}
