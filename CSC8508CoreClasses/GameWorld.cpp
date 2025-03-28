#include "GameWorld.h"
#include "GameObject.h"
#include "Constraint.h"
#include "CollisionDetection.h"
#include "Camera.h"
#include "ComponentManager.h"
#include "PhysicsComponent.h"
#include "TransformNetworkComponent.h"

#include "Transform.h"
#include "IComponent.h"
#include "NetworkBase.h"
#include "Event.h"
#include "EventManager.h"
#include "INetworkComponent.h"

using namespace NCL;
using namespace NCL::CSC8508;

GameWorld::GameWorld()	{
	shuffleConstraints	= false;
	shuffleObjects		= false;
	worldIDCounter		= 0;
	worldStateCounter	= 0;
}

GameWorld::~GameWorld(){}

GameWorld& GameWorld::Instance() {
	static GameWorld instance;
	return instance;
}

void GameWorld::Clear() {
	gameObjects.clear();
	constraints.clear();
	boundsComponents.clear();
	physicsComponents.clear();
	worldIDCounter		= 0;
	worldStateCounter	= 0;
}

void GameWorld::ClearAndErase() {
	for (auto& i : gameObjects)
		delete i;
	for (auto& i : constraints)
		delete i;
	Clear();
}

struct  GameWorld::WorldSaveData : ISerializedData {
	WorldSaveData() : nearPlane(0), farPlane(0), pitch(0), yaw(0), position(Vector3()) {}
	WorldSaveData(float nearPlane, float farPlane, float pitch, float yaw, Vector3 position) :
		nearPlane(nearPlane), farPlane(farPlane), pitch(pitch), yaw(yaw), position(position) {}
	float nearPlane;
	float farPlane;
	float pitch;
	float yaw;
	Vector3 position;
	std::vector<size_t> gameObjectPointers;

	static auto GetSerializedFields() {
		return std::make_tuple(
			SERIALIZED_FIELD(WorldSaveData, nearPlane),
			SERIALIZED_FIELD(WorldSaveData, farPlane),
			SERIALIZED_FIELD(WorldSaveData, pitch),
			SERIALIZED_FIELD(WorldSaveData, yaw),
			SERIALIZED_FIELD(WorldSaveData, position),
			SERIALIZED_FIELD(WorldSaveData, gameObjectPointers)
		);
	}
};

void GameWorld::LoadCameraInfo(float nearPlane, float farPlane, float pitch, float yaw, Vector3 position) {
	GetMainCamera().SetNearPlane(nearPlane);
	GetMainCamera().SetFarPlane(farPlane);
	GetMainCamera().SetPitch(pitch);
	GetMainCamera().SetYaw(yaw);
	GetMainCamera().SetPosition(position);
}

void GameWorld::Load(std::string assetPath, size_t allocationStart) {
	WorldSaveData loadedSaveData = ISerializedData::LoadISerializable<WorldSaveData>(assetPath, allocationStart);
	LoadCameraInfo(loadedSaveData.nearPlane, loadedSaveData.farPlane, 
		loadedSaveData.pitch, loadedSaveData.yaw, loadedSaveData.position);

	for (int i = 0; i < loadedSaveData.gameObjectPointers.size(); i++) {
		GameObject* object = new GameObject(true);
		object->Load(assetPath, loadedSaveData.gameObjectPointers[i]);
		AddGameObject(object);
	}
}

size_t GameWorld::Save(std::string assetPath, size_t* allocationStart)
{
	bool clearMemory = false;
	if (allocationStart == nullptr) {
		allocationStart = new size_t(0);
		clearMemory = true;
	}
	WorldSaveData saveInfo(0.1f, 500.0f, -15.0f, 315.0f, Vector3(-60, 40, 60));

	for (GameObject* gameObject : gameObjects) {
		if (gameObject->HasParent())
			continue;

		size_t nextMemoryLocation = gameObject->Save(assetPath, allocationStart);
		saveInfo.gameObjectPointers.push_back(*allocationStart);
		*allocationStart = nextMemoryLocation;
	}
	SaveManager::GameData saveData = ISerializedData::CreateGameData<WorldSaveData>(saveInfo);
	size_t nextMemoryLocation = SaveManager::SaveGameData(assetPath, saveData, allocationStart);

	if (clearMemory)
		delete allocationStart;
	return nextMemoryLocation;
}

void GameWorld::AddGameObject(GameObject* o) {
	gameObjects.emplace_back(o);
	o->SetWorldID(worldIDCounter++);
	worldStateCounter++;

	auto bounds = o->TryGetComponent<BoundsComponent>();
	auto phys = o->TryGetComponent<PhysicsComponent>();

	if (bounds) boundsComponents.emplace_back(bounds);
	if (phys) physicsComponents.emplace_back(phys);
	auto newComponents = o->GetAllComponents();

	for (IComponent* component : newComponents)
		component->InvokeOnAwake();
	for (GameObject* child : o->GetChildren())
		AddGameObject(child);

	o->InvokeOnAwake();
}

void GameWorld::RemoveGameObject(GameObject* o, bool andDelete) {
	gameObjects.erase(std::remove(gameObjects.begin(), gameObjects.end(), o), gameObjects.end());
	if (andDelete)
		delete o;
	worldStateCounter++;
}

void GameWorld::GetPhysicsIterators(
	PhysicsIterator& first,
	PhysicsIterator& last) const {

	first = physicsComponents.begin();
	last = physicsComponents.end();
}

void GameWorld::GetBoundsIterators(
	BoundsIterator& first,
	BoundsIterator& last) const {

	first = boundsComponents.begin();
	last = boundsComponents.end();
}

void GameWorld::GetObjectIterators(
	GameObjectIterator& first,
	GameObjectIterator& last) const {

	first	= gameObjects.begin();
	last	= gameObjects.end();
}


void GameWorld::OperateOnContents(GameObjectFunc f) {
	for (GameObject* g : gameObjects)
		f(g);
}


void GameWorld::UpdateWorld(float dt){
	if (paused) return;
	ComponentManager::OperateOnAllIComponentBufferOperators(
		[&](IComponent* c) {
			if (c->IsEnabled())
				c->InvokeEarlyUpdate(dt);
		});
	ComponentManager::OperateOnAllIComponentBufferOperators(
		[&](IComponent* c) {
			if (c->IsEnabled()) 
				c->InvokeUpdate(dt);
		});
	OperateOnContents(
		[&](GameObject* o) {
			if (o->IsEnabled()) {
				o->InvokeUpdate(dt);
			}
		});
}

void GameWorld::ShuffleWorldConstraints() {
	auto rng = std::default_random_engine{};
	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine e(seed);

	if (shuffleObjects)
		std::shuffle(gameObjects.begin(), gameObjects.end(), e);

	if (shuffleConstraints)
		std::shuffle(constraints.begin(), constraints.end(), e);
}

bool GameWorld::Raycast(Ray& r, RayCollision& closestCollision, bool closestObject, BoundsComponent* ignoreThis, vector<Layers::LayerID>* ignoreLayers) const {
	RayCollision collision;
	for (auto& i : boundsComponents) {
		if (!i->IsEnabled() || !i->GetBoundingVolume() || i == ignoreThis) continue;
		bool toContinue = false;

		if (ignoreLayers != nullptr) {
			for (const Layers::LayerID& var : *ignoreLayers) {
				if (i->GetGameObject().GetLayerID() == var) {
					toContinue = true;
					break;
				}
			}
		}

		if (toContinue || (i->GetGameObject().GetLayerID() == Layers::Ignore_RayCast || 
			i->GetGameObject().GetLayerID() == Layers::UI))
			continue;

		RayCollision thisCollision;
		if (CollisionDetection::RayIntersection(r, *i, thisCollision)) {
				
			if (!closestObject) {	
				closestCollision = collision;
				closestCollision.node = i;
				return true;
			}
			else {
				if (thisCollision.rayDistance < collision.rayDistance) {
					thisCollision.node = i;
					collision = thisCollision;
				}
			}
		}
	}
	if (collision.node) {
		closestCollision = collision;
		closestCollision.node = collision.node;
		return true;
	}
	return false;
}

void GameWorld::AddConstraint(Constraint* c) {
	constraints.emplace_back(c);
}

void GameWorld::RemoveConstraint(Constraint* c, bool andDelete) {
	constraints.erase(std::remove(constraints.begin(), constraints.end(), c), constraints.end());
	if (andDelete) {
		delete c;
	}
}

void GameWorld::GetConstraintIterators(
	std::vector<Constraint*>::const_iterator& first,
	std::vector<Constraint*>::const_iterator& last) const {
	first	= constraints.begin();
	last	= constraints.end();
}