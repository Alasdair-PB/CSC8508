#include "GameWorld.h"
#include "GameObject.h"
#include "Constraint.h"
#include "CollisionDetection.h"
#include "Camera.h"
#include "ComponentManager.h"


using namespace NCL;
using namespace NCL::CSC8508;

GameWorld::GameWorld()	{
	shuffleConstraints	= false;
	shuffleObjects		= false;
	worldIDCounter		= 0;
	worldStateCounter	= 0;
}

GameWorld::~GameWorld()	{
}

void GameWorld::Clear() {
	gameObjects.clear();
	constraints.clear();
	worldIDCounter		= 0;
	worldStateCounter	= 0;
}

void GameWorld::ClearAndErase() {
	for (auto& i : gameObjects) {
		delete i;
	}
	for (auto& i : constraints) {
		delete i;
	}
	Clear();
}

void GameWorld::AddGameObject(GameObject* o) {
	gameObjects.emplace_back(o);
	o->SetWorldID(worldIDCounter++);
	worldStateCounter++;

	auto bounds = o->TryGetComponent<BoundsComponent>();
	auto phys = o->TryGetComponent<PhysicsComponent>();

	if (bounds)
		boundsComponents.emplace_back(bounds);

	if (phys)
		physicsComponents.emplace_back(phys);

	auto newComponents = o->GetAllComponents();

	for (IComponent* component : newComponents) {
		this->components.push_back(component);
		component->InvokeOnAwake();
	}
	o->InvokeOnAwake();
}

void GameWorld::RemoveGameObject(GameObject* o, bool andDelete) {
	gameObjects.erase(std::remove(gameObjects.begin(), gameObjects.end(), o), gameObjects.end());
	if (andDelete) {
		delete o;
	}
	worldStateCounter++;
}

void GameWorld::GetPhysicsIterators(
	PhysicsIterator& first,
	PhysicsIterator& last) const {

	first = physicsComponents.begin();
	last = physicsComponents.end();
}

void GameWorld::GetINetIterators(
	INetIterator& first,
	INetIterator& last) const {

	first = networkComponents.begin();
	last = networkComponents.end();
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
	for (GameObject* g : gameObjects) {
		f(g);
	}
}


void GameWorld::UpdateWorld(float dt){
	ComponentManager::OperateOnBufferContentsDynamicType<IComponent>(
		[&](IComponent* c) {
			if (c->IsEnabled()) {
				c->InvokeUpdate(dt);
				std::cout << "This happens" << std::endl;

			}
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
		if (!i->GetBoundingVolume()) 
			continue;
		if (i == ignoreThis) 
			continue;
		bool toContinue = false;

		if (ignoreLayers != nullptr) {
			for (const Layers::LayerID& var : *ignoreLayers) {
				if (i->GetGameObject().GetLayerID() == var) {
					toContinue = true;
					break;
				}
			}
		}

		if (toContinue)
			continue;

		if (i->GetGameObject().GetLayerID() == Layers::Ignore_RayCast || i->GetGameObject().GetLayerID() == Layers::UI)
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