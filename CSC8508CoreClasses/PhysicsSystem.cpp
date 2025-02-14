#include "PhysicsSystem.h"
#include "PhysicsObject.h"
#include "GameObject.h"
#include "CollisionDetection.h"
#include "Quaternion.h"

#include "Constraint.h"

#include "Debug.h"
#include "Window.h"
#include <functional>
using namespace NCL;
using namespace CSC8508;

PhysicsSystem::PhysicsSystem(GameWorld& g) : gameWorld(g)	{
	applyGravity	= false;
	useBroadPhase	= false;	
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
		if ((*i).framesLeft == numCollisionFrames) {
			i->a->OnCollisionBegin(i->b);
			i->b->OnCollisionBegin(i->a);
		}

		CollisionDetection::CollisionInfo& in = const_cast<CollisionDetection::CollisionInfo&>(*i);
		in.framesLeft--;

		if ((*i).framesLeft < 0) {
			i->a->OnCollisionEnd(i->b);
			i->b->OnCollisionEnd(i->a);
			i = allCollisions.erase(i);
		}
		else {
			++i;
		}
	}
}

void PhysicsSystem::UpdateObjectAABBs() {
	std::vector<GameObject*>::const_iterator first;
	std::vector<GameObject*>::const_iterator last;
	gameWorld.GetObjectIterators(first, last);
	for (auto i = first; i != last; ++i) {
		(*i)->UpdateBroadphaseAABB();
	}
}

void PhysicsSystem::BasicCollisionDetection() {
	std::vector<GameObject*>::const_iterator first;
	std::vector<GameObject*>::const_iterator last;
	gameWorld.GetObjectIterators(first, last);

	for (auto i = first; i != last; ++i) {
		if ((*i)->GetPhysicsObject() == nullptr) {
			continue;
		}
		for (auto j = i + 1; j != last; ++j) {
			if ((*j)->GetPhysicsObject() == nullptr) {
				continue;
			}
			CollisionDetection::CollisionInfo info;
			if (CollisionDetection::ObjectIntersection(*i, *j, info)) 
			{
				ImpulseResolveCollision(*info.a, *info.b, info.point);
				info.framesLeft = numCollisionFrames;
				allCollisions.insert(info);
			}
		}
	}
}

void PhysicsSystem::ImpulseResolveCollision(GameObject& a, GameObject& b, CollisionDetection::ContactPoint& p) const {

	auto layerID = Layers::Ignore_Collisions;
	auto aLayerID = a.GetLayerID();
	auto bLayerID = b.GetLayerID();

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

	PhysicsObject* physA = a.GetPhysicsObject();
	PhysicsObject* physB = b.GetPhysicsObject();

	Transform& transformA = a.GetTransform();
	Transform& transformB = b.GetTransform();

	float totalMass = physA->GetInverseMass() + physB->GetInverseMass();

	if (totalMass == 0) 
		return; 

	transformA.SetPosition(transformA.GetPosition() - (p.normal * p.penetration * (physA->GetInverseMass() / totalMass)));
	transformB.SetPosition(transformB.GetPosition() + (p.normal * p.penetration * (physB->GetInverseMass() / totalMass)));

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

	float cRestitution = a.GetRestitution() + a.GetRestitution(); 

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
	QuadTree<GameObject*> tree(Vector2(1024, 1024), 7, 6);

	std::vector<GameObject*>::const_iterator first;
	std::vector<GameObject*>::const_iterator last;
	gameWorld.GetObjectIterators(first, last);

	for (auto i = first; i != last; ++i) {
		Vector3 halfSizes;
		if (!(*i)->GetBroadphaseAABB(halfSizes)) {
			continue;
		}
		Vector3 pos = (*i)->GetTransform().GetPosition();
		tree.Insert(*i, pos, halfSizes);
	}
	tree.OperateOnContents([&](std::list<QuadTreeEntry<GameObject*>>& data) 
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
	for (std::set<CollisionDetection::CollisionInfo>::iterator i = broadphaseCollisions.begin(); i != broadphaseCollisions.end(); ++i) {
		CollisionDetection::CollisionInfo info = *i;
		if (CollisionDetection::ObjectIntersection(info.a, info.b, info)) {
			info.framesLeft = numCollisionFrames;
			ImpulseResolveCollision(*info.a, *info.b, info.point);
			allCollisions.insert(info); // insert into our main set
		}
	}
}

void PhysicsSystem::IntegrateAccel(float dt)
{
	std::vector<GameObject*>::const_iterator first;
	std::vector<GameObject*>::const_iterator last;
	gameWorld.GetObjectIterators(first, last);

	for (auto i = first; i != last; ++i) {
		PhysicsObject* object = (*i)->GetPhysicsObject();
		if (object == nullptr) {
			continue; 
		}
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
	std::vector<GameObject*>::const_iterator first;
	std::vector<GameObject*>::const_iterator last;
	gameWorld.GetObjectIterators(first, last);

	float frameLinearDamping = 1.0f - (0.4f * dt);

	for (auto i = first; i != last; ++i) {
		PhysicsObject* object = (*i)->GetPhysicsObject();

		if (object == nullptr) 
			continue;

		Transform& transform = (*i)->GetTransform();

		Vector3 position = transform.GetPosition();
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
	gameWorld.OperateOnContents(
		[](GameObject* o) {
			if (o->GetPhysicsObject())
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