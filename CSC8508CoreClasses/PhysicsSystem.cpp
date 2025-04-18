#include "PhysicsSystem.h"
#include "PhysicsObject.h"
#include "GameObject.h"
#include "PhysicsComponent.h"
#include "BoundsComponent.h"

#include "CollisionDetection.h"
#include "Quaternion.h"

#include "Constraint.h"
#include "ComponentManager.h"

#include "Debug.h"
#include "Window.h"
#include <functional>

#include "CollisionEvent.h"
#include "KDTree/KDTree.h"
using namespace NCL;
using namespace CSC8508;

PhysicsSystem::PhysicsSystem(GameWorld& g) : gameWorld(g)	{
	applyGravity	= false;
	useBroadPhase	= true;
	dTOffset		= 0.0f;
	globalDamping	= 0.995f;
	SetGravity(Vector3(0.0f, -9.8f, 0.0f));
}

PhysicsSystem::~PhysicsSystem()	{
}

void PhysicsSystem::SetGravity(const Vector3& g) {
	gravity = g;
}

void PhysicsSystem::Clear() {
	allCollisions.clear();
}

bool useSimpleContainer = false;

int constraintIterationCount = 10;

const int   idealHZ = 120;
const float idealDT = 1.0f / idealHZ;

int realHZ		= idealHZ;
float realDT	= idealDT;

void PhysicsSystem::DebugConstraints() {
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::B)) {
		useBroadPhase = !useBroadPhase;
		std::cout << "Setting broadphase to " << useBroadPhase << std::endl;
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::N)) {
		useSimpleContainer = !useSimpleContainer;
		std::cout << "Setting broad container to " << useSimpleContainer << std::endl;
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::I)) {
		constraintIterationCount--;
		std::cout << "Setting constraint iterations to " << constraintIterationCount << std::endl;
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::O)) {
		constraintIterationCount++;
		std::cout << "Setting constraint iterations to " << constraintIterationCount << std::endl;
	}
}

void PhysicsSystem::Update(float dt) {	

	dTOffset += dt;

	GameTimer t;
	t.GetTimeDeltaSeconds();

	if (useBroadPhase) 
		UpdateObjectAABBs();
	
	int iteratorCount = 0;

	while(dTOffset > realDT) {
		IntegrateAccel(realDT); 
		if (useBroadPhase) {
			BroadPhase();
			NarrowPhase();
		}
		else 
			BasicCollisionDetection();

		float constraintDt = realDT /  (float)constraintIterationCount;
		for (int i = 0; i < constraintIterationCount; ++i) {
			UpdateConstraints(constraintDt);	
		}
		IntegrateVelocity(realDT);

		dTOffset -= realDT;
		iteratorCount++;
	}

	ClearForces();	
	UpdateCollisionList(); 

	t.Tick();
	float updateTime = t.GetTimeDeltaSeconds();

	if (updateTime > realDT) {
		realHZ /= 2;
		realDT *= 2;
	}
	else if(dt*2 < realDT) { 
		int temp = realHZ;
		realHZ *= 2;
		realDT /= 2;

		if (realHZ > idealHZ) {
			realHZ = idealHZ;
			realDT = idealDT;
		}
	}
}

void PhysicsSystem::UpdateCollisionList() {
	for (std::set<CollisionDetection::CollisionInfo>::iterator i = allCollisions.begin(); i != allCollisions.end(); ) {
		//auto e = CollisionEvent(i->a->GetGameObject(), i->b->GetGameObject(), *i);
		//EventManager::Call(&e); // TODO: I believe this is handled in the collision resolution bit, need to double check

		CollisionDetection::CollisionInfo& in = const_cast<CollisionDetection::CollisionInfo&>(*i);
		in.framesLeft--;

		if ((*i).framesLeft < 0) {
			i = allCollisions.erase(i);
		}
		else {
			++i;
		}
	}
}

void PhysicsSystem::UpdateObjectAABBs() {
	std::vector<BoundsComponent*>::const_iterator first;
	std::vector<BoundsComponent*>::const_iterator last;
	gameWorld.GetBoundsIterators(first, last);
	for (auto i = first; i != last; ++i) {
		if (!(*i)->IsEnabled()) continue;
		(*i)->UpdateBroadphaseAABB();
	}
}

void PhysicsSystem::BasicCollisionDetection() {
	std::vector<BoundsComponent*>::const_iterator first;
	std::vector<BoundsComponent*>::const_iterator last;
	gameWorld.GetBoundsIterators(first, last);

	for (auto i = first; i != last; ++i) {
		if ((*i)->GetPhysicsComponent() == nullptr || !(*i)->IsEnabled()) {
			continue;
		}
		for (auto j = i + 1; j != last; ++j) {
			if ((*j)->GetPhysicsComponent() == nullptr || !(*i)->IsEnabled()) {
				continue;
			}
			CollisionDetection::CollisionInfo info;
			if (CollisionDetection::ObjectIntersection(*i, *j, info))
			{
				auto e = CollisionEvent((*i)->GetGameObject(), (*j)->GetGameObject(), info);
				EventManager::Call(&e);
				ImpulseResolveCollision(*info.a, *info.b, info.point);
				info.framesLeft = numCollisionFrames;
				allCollisions.insert(info);
			}
		}
	}
}

void PhysicsSystem::ImpulseResolveCollision(BoundsComponent& a, BoundsComponent& b, CollisionDetection::ContactPoint& p) const {
	GameObject& aObject = a.GetGameObject();
	GameObject& bObject = b.GetGameObject();

	auto layerID = Layers::Ignore_Collisions;
	auto aLayerID = aObject.GetLayerID();
	auto bLayerID = bObject.GetLayerID();

	if (aLayerID == layerID || bLayerID == layerID)
		return;

	if (a.GetBoundingVolume()->isTrigger || b.GetBoundingVolume()->isTrigger)
		return;

	for (const auto& layer : a.GetIgnoredLayers()) {
		if (bLayerID == layer)
			return;
	}

	for (const auto& layer : b.GetIgnoredLayers()) {
		if (aLayerID == layer)
			return;
	}

	PhysicsObject* physA = a.GetPhysicsComponent()->GetPhysicsObject();
	PhysicsObject* physB = b.GetPhysicsComponent()->GetPhysicsObject();

	Transform& transformA = aObject.GetTransform();
	Transform& transformB = bObject.GetTransform();

	float totalMass = physA->GetInverseMass() + physB->GetInverseMass();

	if (totalMass == 0) 
		return; 
	if (physA->GetInverseMass() != 0)
		transformA.SetPosition(transformA.GetLocalPosition() - (p.normal * p.penetration * (physA->GetInverseMass() / totalMass)));

	if (physB->GetInverseMass() != 0)
		transformB.SetPosition(transformB.GetLocalPosition() + (p.normal * p.penetration * (physB->GetInverseMass() / totalMass)));

	Vector3 relativeA = p.localA;
	Vector3 relativeB = p.localB;

	Vector3 angVelocityA = Vector::Cross(physA->GetAngularVelocity(), relativeA);
	Vector3 angVelocityB = Vector::Cross(physB->GetAngularVelocity(), relativeB);

	Vector3 fullVelocityA = physA->GetLinearVelocity() + angVelocityA;
	Vector3 fullVelocityB = physB->GetLinearVelocity() + angVelocityB;

	Vector3 contactVelocity = fullVelocityB - fullVelocityA;	
	float cFriction = (physA->GetFriction() + physB->GetFriction()) / 2;
	contactVelocity *= cFriction;
	float impulseForce = Vector::Dot(contactVelocity, p.normal);

	Vector3 inertiaA = Vector::Cross(physA->GetInertiaTensor() * Vector::Cross(relativeA, p.normal), relativeA);
	Vector3 inertiaB = Vector::Cross(physB->GetInertiaTensor() * Vector::Cross(relativeB, p.normal), relativeB);
	float angularEffect = Vector::Dot(inertiaA + inertiaB, p.normal);

	float cRestitution = physA->GetRestitution() + physB->GetRestitution();

	if (cRestitution > 0)
		cRestitution /= 2;

	float j = (-(1.0f + cRestitution) * impulseForce) / (totalMass + angularEffect);


	Vector3 fullImpulse = (p.normal * j);

	physA->ApplyLinearImpulse(-fullImpulse);
	physB->ApplyLinearImpulse(fullImpulse);

	physA->ApplyAngularImpulse(Vector::Cross(relativeA, -fullImpulse));
	physB->ApplyAngularImpulse(Vector::Cross(relativeB, fullImpulse));
}


void PhysicsSystem::BroadPhase() {
	broadphaseCollisions.clear();
	QuadTree<BoundsComponent*> dynTree(Vector2(1024, 1024), 7, 6);
	QuadTree<BoundsComponent*> staticTree(Vector2(1024, 1024), 7, 6);
	//auto staticTree = KDTree<BoundsComponent>();

	std::vector<BoundsComponent*>::const_iterator first;
	std::vector<BoundsComponent*>::const_iterator last;
	gameWorld.GetBoundsIterators(first, last);

	// Make the trees
	for (auto i = first; i != last; ++i) {
		Vector3 halfSizes;
		if (!(*i)->GetBroadphaseAABB(halfSizes) || !(*i)->IsEnabled()) {
			continue;
		}
		Vector3 pos = (*i)->GetGameObject().GetTransform().GetPosition();
		if ((*i)->GetGameObject().IsStatic()) staticTree.Insert(*i, pos, halfSizes);
		else dynTree.Insert(*i, pos, halfSizes);
	}

	// // Check for static collisions
	// for (auto i = first; i != last; ++i) {
	// 	if ((*i)->GetGameObject().IsStatic()) return;
	//
	// 	Vector3 halfSizes;
	// 	if (!(*i)->GetBroadphaseAABB(halfSizes) || !(*i)->IsEnabled()) {
	// 		continue;
	// 	}
	// 	Vector3 pos = (*i)->GetGameObject().GetTransform().GetPosition();
	// 	auto query = KDTreeQuery(pos, halfSizes);
	// 	//auto out = kdTree.Get(query);
	// 	for (KDTreeEntry<BoundsComponent> e : staticTree.Get(query)) {
	// 		CollisionDetection::CollisionInfo info;
	// 		info.a = std::min(*i, e.value);
	// 		info.b = std::max(*i, e.value);
	// 		broadphaseCollisions.insert(info);
	// 	}
	// }

	// Check non-static
	staticTree.OperateOnContents([&](std::list<QuadTreeEntry<BoundsComponent*>>& staticData)
	{
		dynTree.OperateOnContents([&](std::list<QuadTreeEntry<BoundsComponent*>>& dynData) {
			for (auto i = staticData.begin(); i != staticData.end(); ++i)
		{
			for (auto j = dynData.begin(); j != dynData.end(); ++j)
			{
				CollisionDetection::CollisionInfo info;
				info.a = std::min((*i).object, (*j).object);
				info.b = std::max((*i).object, (*j).object);
				broadphaseCollisions.insert(info);
			}
		}
		});
	});

	// Check non-static
	dynTree.OperateOnContents([&](std::list<QuadTreeEntry<BoundsComponent*>>& data)
	{

		CollisionDetection::CollisionInfo info;
		for (auto i = data.begin(); i != data.end(); ++i) 
		{
			for (auto j = std::next(i); j != data.end(); ++j) 
			{
				info.a = std::min((*i).object, (*j).object);
				info.b = std::max((*i).object, (*j).object);
				broadphaseCollisions.insert(info);
			}
		}
	});
}

void PhysicsSystem::NarrowPhase() {
	int collisionTestCount = 0;
	for (std::set<CollisionDetection::CollisionInfo>::iterator i = broadphaseCollisions.begin(); i != broadphaseCollisions.end(); ++i) {
		CollisionDetection::CollisionInfo info = *i;
		collisionTestCount++;
		if (CollisionDetection::ObjectIntersection(info.a, info.b, info)) {
			auto e = CollisionEvent(info.a->GetGameObject(), info.b->GetGameObject(), info);
			EventManager::Call(&e);
			info.framesLeft = numCollisionFrames;
			ImpulseResolveCollision(*info.a, *info.b, info.point);
			allCollisions.insert(info); // insert into our main set
		}
	}
}

void PhysicsSystem::IntegrateAccel(float dt)
{
	std::vector<PhysicsComponent*>::const_iterator first;
	std::vector<PhysicsComponent*>::const_iterator last;
	gameWorld.GetPhysicsIterators(first, last);

	for (auto i = first; i != last; ++i) {
		PhysicsObject* object = (*i)->GetPhysicsObject();
		if (object == nullptr || !(*i)->IsEnabled())
			continue; 
		float inverseMass = object->GetInverseMass();

		Vector3 linearVel = object->GetLinearVelocity();
		Vector3 force = object->GetForce();
		Vector3 accel = force * inverseMass;

		if (applyGravity && inverseMass > 0) 
			accel += gravity; 
		

		linearVel += accel * dt; 
		object->SetLinearVelocity(linearVel);

		Vector3 torque = object ->GetTorque();
		Vector3 angVel = object->GetAngularVelocity();

		object->UpdateInertiaTensor();

		Vector3 angAccel = object->GetInertiaTensor() * torque;
		angVel += angAccel * dt;
		object->SetAngularVelocity(angVel);
	}
}

void PhysicsSystem::IntegrateVelocity(float dt) {
	std::vector<PhysicsComponent*>::const_iterator first;
	std::vector<PhysicsComponent*>::const_iterator last;
	gameWorld.GetPhysicsIterators(first, last);

	float frameLinearDamping = 1.0f - (0.4f * dt);

	for (auto i = first; i != last; ++i) {
		PhysicsObject* object = (*i)->GetPhysicsObject();

		if (object == nullptr || !(*i)->IsEnabled()) 
			continue;

		if (object->GetInverseMass() == 0)
			continue;

		Transform& transform = (*i)->GetGameObject().GetTransform();

		Vector3 position = transform.GetLocalPosition();
		Vector3 linearVel = object->GetLinearVelocity();
		position += linearVel * dt;
		transform.SetPosition(position);

		linearVel = linearVel * frameLinearDamping;
		//linearVel *= object->GetFriction();

		object->SetLinearVelocity(linearVel);

		Quaternion orientation = transform.GetOrientation();
		Vector3 angVel = object->GetAngularVelocity();

		orientation = orientation +
			(Quaternion(angVel * dt * 0.5f, 0.0f) * orientation);
		orientation.Normalise();

		transform.SetOrientation(orientation);

		float frameAngularDamping = 1.0f - (0.4f * dt);
		angVel = angVel * frameAngularDamping;
		object->SetAngularVelocity(angVel);

	}
}

void PhysicsSystem::ClearForces() {
	ComponentManager::OperateOnBufferContents<PhysicsComponent>(
		[](PhysicsComponent* o) {
			if (o->IsEnabled() && o->GetPhysicsObject())
				o->GetPhysicsObject()->ClearForces();
		}
	);
}

void PhysicsSystem::UpdateConstraints(float dt) {
	std::vector<Constraint*>::const_iterator first;
	std::vector<Constraint*>::const_iterator last;
	gameWorld.GetConstraintIterators(first, last);

	for (auto i = first; i != last; ++i) {
		(*i)->UpdateConstraint(dt);
	}
}