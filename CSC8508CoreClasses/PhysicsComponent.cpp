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
