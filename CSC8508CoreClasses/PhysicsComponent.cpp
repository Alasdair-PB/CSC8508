#include "PhysicsComponent.h"
#include "CollisionDetection.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "NetworkObject.h"
#include "INetworkDeltaComponent.h"


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

void PhysicsComponent::SetInitType(InitType type, PhysicsObject* phyObj) {
	switch (type)
	{
	case(Sphere): {
		phyObj->InitSphereInertia();
		break;
	}
	case(Cube): {
		phyObj->InitCubeInertia();
		break;
	}
	default:
		break;
	}
}

void PhysicsComponent::CopyComponent(GameObject* gameObject) {

	PhysicsComponent* component = gameObject->AddComponent<PhysicsComponent>();
	component->SetEnabled(IsEnabled());

	PhysicsObject* physObjCopy = new PhysicsObject(&gameObject->GetTransform());
	physObjCopy->SetInverseMass(physicsObject->GetInverseMass());
	physObjCopy->SetFriction(physicsObject->GetFriction());
	physObjCopy->SetRestitution(physicsObject->GetCRestitution());
	component->SetPhysicsObject(physObjCopy);
	SetInitType(initiType, physObjCopy);
}

auto PhysicsComponent::GetDerivedSerializedFields() const {
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
	SetInitType(loadedSaveData.initType, physicsObject);
}

void PhysicsComponent::PushIComponentElementsInspector(UIElementsGroup& elementsGroup, float scale) {
	IComponent::PushIComponentElementsInspector(elementsGroup, scale);

	if (physicsObject){
		elementsGroup.PushFloatElement(physicsObject->GetcRestRef(), scale, "CRestitution:");
		elementsGroup.PushFloatElement(physicsObject->GetFrictionRef(), scale, "Friction:");
		elementsGroup.PushFloatElement(physicsObject->GetInverseMassRef(), scale, "InverseMass:");
	}
	else {
		PhysicsComponent* phys = this;
		elementsGroup.PushStatelessButtonElement(ImVec2(scale, scale), "AddPhysicsObject", 
			[phys](){phys->SetPhysicsObject(new PhysicsObject(&phys->GetGameObject().GetTransform()));});
	}
	std::set<std::pair<int*, std::string>> enumOptions = {
		{reinterpret_cast<int*>(&initiType), "None"},
		{reinterpret_cast<int*>(&initiType), "Sphere"},
		{reinterpret_cast<int*>(&initiType), "Cube"}
	};
	elementsGroup.PushEnumElement("Select Option", enumOptions);
}
