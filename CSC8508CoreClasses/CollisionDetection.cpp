//
// Original author: Rich Davison
// Contributors: Alasdair, Alfie
//

#include "CollisionDetection.h"

#include <Util.cpp>

#include "CollisionVolume.h"
#include "AABBVolume.h"
#include "Axis.h"
#include "OBBVolume.h"
#include "SphereVolume.h"
#include "Window.h"
#include "Maths.h"

using namespace NCL;

bool CollisionDetection::RayPlaneIntersection(const Ray&r, const Plane&p, RayCollision& collisions) {
	float ln = Vector::Dot(p.GetNormal(), r.GetDirection());

	if (ln == 0.0f) {
		return false; //direction vectors are perpendicular!
	}
	
	Vector3 planePoint = p.GetPointOnPlane();
	Vector3 pointDir = planePoint - r.GetPosition();

	float d = Vector::Dot(pointDir, p.GetNormal()) / ln;

	collisions.collidedAt = r.GetPosition() + (r.GetDirection() * d);

	return true;
}

bool CollisionDetection::RayIntersection(const Ray& r, BoundsComponent& object, RayCollision& collision) {
	bool hasCollided = false;

	const Transform& worldTransform = object.GetGameObject().GetTransform();
	const CollisionVolume* volume	= object.GetBoundingVolume();

	if (!volume) {
		return false;
	}

	switch (volume->type) {
		case VolumeType::AABB:		hasCollided = RayAABBIntersection(r, worldTransform, (const AABBVolume&)*volume	, collision); break;
		case VolumeType::OBB:		hasCollided = RayOBBIntersection(r, worldTransform, (const OBBVolume&)*volume	, collision); break;
		case VolumeType::Sphere:	hasCollided = RaySphereIntersection(r, worldTransform, (const SphereVolume&)*volume	, collision); break;
		case VolumeType::Capsule:	hasCollided = RayCapsuleIntersection(r, worldTransform, (const CapsuleVolume&)*volume, collision); break;
	}

	return hasCollided;
}



bool CollisionDetection::RayBoxIntersection(const Ray& r, const Vector3& boxPos, const Vector3& boxSize, RayCollision& collision) {
	Vector3 boxMin = boxPos - boxSize;
	Vector3 boxMax = boxPos + boxSize;

	Vector3 rayPos = r.GetPosition();
	Vector3 rayDir = r.GetDirection();

	Vector3 tVals(-1, -1, -1);

	for (int i = 0; i < 3; ++i) { // get best 3 intersections
		if (rayDir[i] > 0) {
			tVals[i] = (boxMin[i] - rayPos[i]) / rayDir[i];
		}
		else if (rayDir[i] < 0) {
			tVals[i] = (boxMax[i] - rayPos[i]) / rayDir[i];
		}
	}
	float bestT = Vector::GetMaxElement(tVals); //tVals.GetMaxElement();
	if (bestT < 0.0f) {
		return false; // no backwards rays!
	}
	Vector3 intersection = rayPos + (rayDir * bestT);
	const float epsilon = 0.0001f; // an amount of leeway in our calcs
	for (int i = 0; i < 3; ++i) {
		if (intersection[i] + epsilon < boxMin[i] ||
			intersection[i] - epsilon > boxMax[i]) {
			return false; // best intersection doesn �t touch the box !
		}
	}
	collision.collidedAt = intersection;
	collision.rayDistance = bestT;
	return true;
}

bool CollisionDetection::RayAABBIntersection(const Ray& r, const Transform & worldTransform, const AABBVolume & volume, RayCollision& collision) 
{
	Vector3 boxPos = worldTransform.GetPosition();
	Vector3 boxSize = volume.GetHalfDimensions();
	return RayBoxIntersection(r, boxPos, boxSize, collision);
}

bool CollisionDetection::RayOBBIntersection(const Ray& r, const Transform& worldTransform, const OBBVolume& volume, RayCollision& collision) 
{
	Quaternion orientation = worldTransform.GetOrientation();
	Vector3 position = worldTransform.GetPosition();

	Matrix3 transform = Quaternion::RotationMatrix<Matrix3>(orientation); 
	Matrix3 invTransform = Quaternion::RotationMatrix<Matrix3>(orientation.Conjugate());

	Vector3 localRayPos = r.GetPosition() - position;
	Ray tempRay(invTransform * localRayPos, invTransform * r.GetDirection());

	bool collided = RayBoxIntersection(tempRay, Vector3(), volume.GetHalfDimensions(), collision);

	if (collided) {
		collision.collidedAt = transform * collision.collidedAt + position;
	}
	return collided;
}


bool CollisionDetection::RaySphereIntersection(const Ray& r, const Transform& worldTransform, const SphereVolume& volume, RayCollision& collision) {
	Vector3 spherePos = worldTransform.GetPosition();
	float sphereRadius = volume.GetRadius();

	Vector3 dir = (spherePos - r.GetPosition());
	float sphereProj = Vector::Dot(dir, r.GetDirection());

	if (sphereProj < 0.0f) {
		return false;
	}

	Vector3 point = r.GetPosition() + (r.GetDirection() * sphereProj);
	float sphereDist = Vector::Length(point - spherePos);

	if (sphereDist > sphereRadius) {
		return false;
	}

	float offset = sqrt((sphereRadius * sphereRadius) - (sphereDist * sphereDist));
	collision.rayDistance = sphereProj - offset;
	collision.collidedAt = r.GetPosition() + (r.GetDirection() * collision.rayDistance);
	return true;
}

bool CollisionDetection::RayCapsuleIntersection(const Ray& r, const Transform& worldTransform, const CapsuleVolume& volume, RayCollision& collision) {
	return false;
}

void GetChildBoundsComponent(GameObject* gameObject, std::vector<BoundsComponent*>& out) {
	for (GameObject* c : gameObject->GetChildren()) {
		if (auto* boundsComponent = c->TryGetComponent<BoundsComponent>()) out.push_back(boundsComponent);
		GetChildBoundsComponent(c, out);
	}
}

bool CollisionDetection::ObjectIntersection(GameObject* gameObjectA, GameObject* gameObjectB, CollisionInfo& collisionInfo) {
	// Gather all bounds components that need checking
	std::vector<BoundsComponent*> aBounds, bBounds;
	if (auto* aBoundsComponent = gameObjectA->TryGetComponent<BoundsComponent>()) aBounds.push_back(aBoundsComponent);
	if (auto* bBoundsComponent = gameObjectB->TryGetComponent<BoundsComponent>()) bBounds.push_back(bBoundsComponent);
	GetChildBoundsComponent(gameObjectA, aBounds);
	GetChildBoundsComponent(gameObjectB, bBounds);

	// Check for collision
	for (BoundsComponent* a : aBounds) for (BoundsComponent* b : bBounds)
		if (ObjectIntersection(a, b, collisionInfo)) return true;
	return false;
}

bool CollisionDetection::ObjectIntersection(BoundsComponent* a, BoundsComponent* b, CollisionInfo& collisionInfo) {
	const CollisionVolume* volA = a->GetBoundingVolume();
	const CollisionVolume* volB = b->GetBoundingVolume();

	if (!volA || !volB)
		return false;

	collisionInfo.a = a;
	collisionInfo.b = b;

	Transform& transformA = a->GetGameObject().GetTransform();
	Transform& transformB = b->GetGameObject().GetTransform();

	VolumeType pairType = (VolumeType)((int)volA->type | (int)volB->type);

	//Two AABBs
	if (pairType == VolumeType::AABB) {
		return AABBIntersection((AABBVolume&)*volA, transformA, (AABBVolume&)*volB, transformB, collisionInfo);
	}
	//Two Spheres
	if (pairType == VolumeType::Sphere) {
		return SphereIntersection((SphereVolume&)*volA, transformA, (SphereVolume&)*volB, transformB, collisionInfo);
	}
	//Two OBBs
	if (pairType == VolumeType::OBB) {
		return OBBIntersection((OBBVolume&)*volA, transformA, (OBBVolume&)*volB, transformB, collisionInfo);
	}
	//Two Capsules

	//AABB vs Sphere pairs
	if (volA->type == VolumeType::AABB && volB->type == VolumeType::Sphere) {
		return AABBSphereIntersection((AABBVolume&)*volA, transformA, (SphereVolume&)*volB, transformB, collisionInfo);
	}
	if (volA->type == VolumeType::Sphere && volB->type == VolumeType::AABB) {
		collisionInfo.a = b;
		collisionInfo.b = a;
		return AABBSphereIntersection((AABBVolume&)*volB, transformB, (SphereVolume&)*volA, transformA, collisionInfo);
	}

	//OBB vs sphere pairs
	if (volA->type == VolumeType::OBB && volB->type == VolumeType::Sphere) {
		return OBBSphereIntersection((OBBVolume&)*volA, transformA, (SphereVolume&)*volB, transformB, collisionInfo);
	}
	if (volA->type == VolumeType::Sphere && volB->type == VolumeType::OBB) {
		collisionInfo.a = b;
		collisionInfo.b = a;
		return OBBSphereIntersection((OBBVolume&)*volB, transformB, (SphereVolume&)*volA, transformA, collisionInfo);
	}

	//Capsule vs other interactions
	if (volA->type == VolumeType::Capsule && volB->type == VolumeType::Sphere) {
		return SphereCapsuleIntersection((CapsuleVolume&)*volA, transformA, (SphereVolume&)*volB, transformB, collisionInfo);
	}
	if (volA->type == VolumeType::Sphere && volB->type == VolumeType::Capsule) {
		collisionInfo.a = b;
		collisionInfo.b = a;
		return SphereCapsuleIntersection((CapsuleVolume&)*volB, transformB, (SphereVolume&)*volA, transformA, collisionInfo);
	}

	if (volA->type == VolumeType::Capsule && volB->type == VolumeType::OBB) {		
		collisionInfo.a = b;
		collisionInfo.b = a;
		return OBBCapsuleIntersection((CapsuleVolume&)*volA, transformA, (OBBVolume&)*volB, transformB, collisionInfo);
	}
	if (volB->type == VolumeType::Capsule && volA->type == VolumeType::OBB) {
		return OBBCapsuleIntersection((CapsuleVolume&)*volB, transformB, (OBBVolume&)*volA, transformA, collisionInfo);
	}

	return false;
}

bool CollisionDetection::AABBTest(const Vector3& posA, const Vector3& posB, const Vector3& halfSizeA, const Vector3& halfSizeB) {
	Vector3 delta = posB - posA;
	Vector3 totalSize = halfSizeA + halfSizeB;

	if (abs(delta.x) < totalSize.x &&
		abs(delta.y) < totalSize.y &&
		abs(delta.z) < totalSize.z) {
		return true;
	}
	return false;
}

//AABB/AABB Collisions
bool CollisionDetection::AABBIntersection(
	const AABBVolume& volumeA, const Transform& worldTransformA,
	const AABBVolume& volumeB, const Transform& worldTransformB,
	CollisionInfo& collisionInfo) {

	Vector3 boxAPos = worldTransformA.GetPosition();
	Vector3 boxBPos = worldTransformB.GetPosition();

	Vector3 boxASize = volumeA.GetHalfDimensions();
	Vector3 boxBSize = volumeB.GetHalfDimensions();

	bool overlap = AABBTest(boxAPos, boxBPos, boxASize, boxBSize);

	if (overlap) {
		static const Vector3 faces[6] = {
			Vector3(-1, 0, 0), Vector3(1, 0, 0),
			Vector3(0, -1, 0), Vector3(0, 1, 0),
			Vector3(0, 0, -1), Vector3(0, 0, 1)
		};

		Vector3 maxA = boxAPos + boxASize;
		Vector3 minA = boxAPos - boxASize;

		Vector3 maxB = boxBPos + boxBSize;
		Vector3 minB = boxBPos - boxBSize;

		float distances[6] = {
			(maxB.x - minA.x),  // distance of box 'b' to 'left' of 'a'.
			(maxA.x - minB.x),  // distance of box 'b' to 'right' of 'a'.
			(maxB.y - minA.y),  // distance of box 'b' to 'bottom' of 'a'.
			(maxA.y - minB.y),  // distance of box 'b' to 'top' of 'a'.
			(maxB.z - minA.z),  // distance of box 'b' to 'far' of 'a'.
			(maxA.z - minB.z)   // distance of box 'b' to 'near' of 'a'.
		};

		float penetration = FLT_MAX;
		Vector3 bestAxis;

		for (int i = 0; i < 6; i++) {
			if (distances[i] < penetration) {
				penetration = distances[i];
				bestAxis = faces[i];
			}
		}

		collisionInfo.AddContactPoint(Vector3(), Vector3(), bestAxis, penetration);
		return true;
	}

	return false;
}

bool CollisionDetection::OBBIntersection(
	const OBBVolume& volumeA, const Transform& worldTransformA,
	const OBBVolume& volumeB, const Transform& worldTransformB,
	CollisionInfo& collisionInfo) {

	// Get all vertices
	Vector3 aVertices[8], bVertices[8];
	Util::GetAllOBBVertices(aVertices, worldTransformA, volumeA.GetHalfDimensions());
	Util::GetAllOBBVertices(bVertices, worldTransformB, volumeB.GetHalfDimensions());

	// Translate vertices to be local to the opposite shape
	for (int i = 0; i < 8; i++) {
		aVertices[i] = worldTransformB.GetOrientation().Conjugate() * (aVertices[i] - worldTransformB.GetPosition());
		bVertices[i] = worldTransformA.GetOrientation().Conjugate() * (bVertices[i] - worldTransformA.GetPosition());
	}

	// Find min and max x, y and z in each vertex array
	auto minA = Vector3(FLT_MAX, FLT_MAX, FLT_MAX);
	auto maxA = Vector3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	auto minB = Vector3(FLT_MAX, FLT_MAX, FLT_MAX);
	auto maxB = Vector3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	for (int i = 0; i < 8; i++) { // For each vertex in vertex arrays
		for (Axis a = x; a <= z; a++) { // For each axis
			if (aVertices[i][a] < minA[a]) minA[a] = aVertices[i][a];
			if (aVertices[i][a] > maxA[a]) maxA[a] = aVertices[i][a];
			if (bVertices[i][a] < minB[a]) minB[a] = bVertices[i][a];
			if (bVertices[i][a] > maxB[a]) maxB[a] = bVertices[i][a];
		}
	}

	// Check that their bounds lie within the half dimensions of the opposing shape
	Vector3 aHalfDimensions = volumeA.GetHalfDimensions();
	Vector3 bHalfDimensions = volumeB.GetHalfDimensions();
	float minPenetration = FLT_MAX;
	int axis = 0;
	for (Axis a = x; a <= z; a++) { // For each axis
		if (minA[a] > bHalfDimensions[a] || maxA[a] < -bHalfDimensions[a]) return false;
		float hold = std::min(bHalfDimensions[a] - minA[a], maxA[a] + bHalfDimensions[a]);
		if (hold < minPenetration) {
			minPenetration = hold;
			axis = a;
		}

		if (minB[a] > aHalfDimensions[a] || maxB[a] < -aHalfDimensions[a]) return false;
		hold = std::min(aHalfDimensions[a] - minB[a], maxB[a] + aHalfDimensions[a]);
		if (hold < minPenetration) {
			minPenetration = hold;
			axis = -a;
		}
	}

	// Find the point of collision
	Vector3 pointOfCollision;
	if (axis > 0) pointOfCollision = (maxA + minA) / 2.0f;
	else if (axis < 0) pointOfCollision = (maxB + minB) / 2.0f;

	float magnitude = axis > 0 ? 1 : -1;
	auto collisionNormal = Vector3(
		abs(axis) % 3 == 0 ? magnitude : 0.0f,
		abs(axis) % 3 == 1 ? magnitude : 0.0f,
		abs(axis) % 3 == 2 ? magnitude : 0.0f
		);

	Vector3 localA, localB, worldPos;
	if (axis > 0) {
		localB = Vector::Clamp(pointOfCollision, -volumeB.GetHalfDimensions(), volumeB.GetHalfDimensions());
		worldPos = worldTransformB.GetOrientation() * localB + worldTransformB.GetPosition();
		localA = worldTransformA.GetOrientation().Conjugate() * (worldPos - worldTransformA.GetPosition());
		localA = Vector::Clamp(localA, -volumeA.GetHalfDimensions(), volumeA.GetHalfDimensions());
		collisionNormal = worldTransformB.GetOrientation() * collisionNormal;
	} else {
		localA = Vector::Clamp(pointOfCollision, -volumeA.GetHalfDimensions(), volumeA.GetHalfDimensions());
		worldPos = worldTransformA.GetOrientation() * localA + worldTransformA.GetPosition();
		localB = worldTransformB.GetOrientation().Conjugate() * (worldPos - worldTransformB.GetPosition());
		localB = Vector::Clamp(localB, -volumeB.GetHalfDimensions(), volumeB.GetHalfDimensions());
		collisionNormal = worldTransformA.GetOrientation() * collisionNormal;
	}

	if (Vector::Dot(collisionNormal, worldTransformB.GetPosition() - worldTransformA.GetPosition()) < 0) { collisionNormal = -collisionNormal; }

	collisionInfo.AddContactPoint(localA, localB, collisionNormal, minPenetration);
	return true;
}


//Sphere / Sphere Collision
bool CollisionDetection::SphereIntersection(
	const SphereVolume& volumeA, const Transform& worldTransformA,
	const SphereVolume& volumeB, const Transform& worldTransformB,
	CollisionInfo& collisionInfo) {

	float radii = volumeA.GetRadius() + volumeB.GetRadius();
	Vector3 delta = worldTransformB.GetPosition() - worldTransformA.GetPosition();

	float deltaLength = Vector::Length(delta);

	if (deltaLength < radii) {
		float penetration = (radii - deltaLength);
		Vector3 normal = Vector::Normalise(delta);
		Vector3 localA = normal * volumeA.GetRadius();
		Vector3 localB = -normal * volumeB.GetRadius();

		collisionInfo.AddContactPoint(localA, localB, normal, penetration);
		return true; // We're colliding!
	}
	return false;
}

bool CollisionDetection::AABBSphereIntersection(
	const AABBVolume& volumeA, const Transform& worldTransformA,
	const SphereVolume& volumeB, const Transform& worldTransformB,
	CollisionInfo& collisionInfo) {

	Vector3 boxSize = volumeA.GetHalfDimensions();
	Vector3 delta = worldTransformB.GetPosition() - worldTransformA.GetPosition();
	Vector3 negativeBoxSize = -boxSize;

	Vector3 closestPointOnBox = Vector::Clamp(delta, negativeBoxSize, boxSize);
	Vector3 localPoint = delta - closestPointOnBox;
	float distance = Vector::Length(localPoint);

	if (distance < volumeB.GetRadius()) { // Yes, we're colliding!
		Vector3 collisionNormal = Vector::Normalise(localPoint);
		float penetration = (volumeB.GetRadius() - distance);

		Vector3 localA = Vector3();
		Vector3 localB = -collisionNormal * volumeB.GetRadius();

		collisionInfo.AddContactPoint(localA, localB, collisionNormal, penetration);
		return true;
	}
	return false;
}

bool CollisionDetection::AABBCapsuleIntersection(
	const CapsuleVolume& volumeA, const Transform& worldTransformA,
	const AABBVolume& volumeB, const Transform& worldTransformB,
	CollisionInfo& collisionInfo) {

	// OBB done instead AABBs don't currently exist in game
	Vector3 capsuleStart = worldTransformA.GetPosition() + (worldTransformA.GetOrientation() * Vector3(0, -volumeA.GetHalfHeight(), 0));
	Vector3 capsuleEnd = worldTransformA.GetPosition() + (worldTransformA.GetOrientation() * Vector3(0, volumeA.GetHalfHeight(), 0));

	Vector3 boxMin = worldTransformB.GetPosition() - volumeB.GetHalfDimensions();
	Vector3 boxMax = worldTransformB.GetPosition() + volumeB.GetHalfDimensions();
	Vector3 closestPointInBox = Vector::Clamp(worldTransformA.GetPosition(), boxMin, boxMax);

	Vector3 closestPoint = Vector::ClosestPointOnLineSegment(capsuleStart, capsuleEnd, closestPointInBox);
	Vector3 delta = closestPoint - closestPointInBox;

	float distance = Vector::Length(delta);
	if (distance < volumeA.GetRadius()) {
		Vector3 normal = Vector::Normalise(delta);
		float penetration = (volumeA.GetRadius() - distance);

		collisionInfo.AddContactPoint(Vector3(), -normal * volumeA.GetRadius(), normal, penetration);
		return true;
	}
	return false;
}

bool CollisionDetection::OBBSphereIntersection(
	const OBBVolume& volumeA, const Transform& worldTransformA,
	const SphereVolume& volumeB, const Transform& worldTransformB, CollisionInfo& collisionInfo)
{
	Matrix3 orientation = Quaternion::RotationMatrix<Matrix3>(worldTransformA.GetOrientation());
	Vector3 boxSize = volumeA.GetHalfDimensions();
	Vector3 delta = worldTransformB.GetPosition() - worldTransformA.GetPosition();

	Vector3 localPoint = Matrix::Transpose(orientation) * delta;
	Vector3 negativeBoxSize = -boxSize;

	Vector3 closestPointOnBox = Vector::Clamp(localPoint, negativeBoxSize, boxSize);
	Vector3 localToSphere = localPoint - closestPointOnBox;
	float distance = Vector::Length(localToSphere);

	if (distance < volumeB.GetRadius())
	{
		Vector3 collisionNormal = orientation * Vector::Normalise(localToSphere);
		float penetration = volumeB.GetRadius() - distance;

		Vector3 localA = orientation * closestPointOnBox;
		Vector3 localB = -collisionNormal * volumeB.GetRadius();

		collisionInfo.AddContactPoint(localA, localB, collisionNormal, penetration);
		return true;
	}
	return false;
}

bool CollisionDetection::OBBCapsuleIntersection(
	const CapsuleVolume& volumeA, const Transform& worldTransformA,
	const OBBVolume& volumeB, const Transform& worldTransformB,
	CollisionInfo& collisionInfo) {
	// All local space variables use the suffix "local". Otherwise, it's either world space or applicable to both

	// Orientation matrix of the OBB to world-space (Used to translate between WS and LS)
	auto orientation = Quaternion::RotationMatrix<Matrix3>(worldTransformB.GetOrientation());
	// Half dimensions of the OBB
	Vector3 boxHalfDimensions = volumeB.GetHalfDimensions();
	// Difference from the capsule's origin to the OBB's origin
	Vector3 delta = worldTransformA.GetPosition() - worldTransformB.GetPosition();
	// Capsule's origin in local space
	Vector3 localCapsuleOrigin = Matrix::Transpose(orientation) * delta;
	// Closest point vector in the OBB volume to the capsule in local space
	Vector3 localClosestPointOnBox = Vector::Clamp(localCapsuleOrigin, -boxHalfDimensions, boxHalfDimensions);

	// World space capsule start and end positions
	Vector3 capsuleStart = worldTransformA.GetPosition() + worldTransformA.GetOrientation() * Vector3(0, -volumeA.GetHalfHeight(), 0);
	Vector3 capsuleEnd = worldTransformA.GetPosition() + worldTransformA.GetOrientation() * Vector3(0, volumeA.GetHalfHeight(), 0);
	// Local space capsule start and end positions
	Vector3 localCapsuleStart = Matrix::Transpose(orientation) * (capsuleStart - worldTransformB.GetPosition());
	Vector3 localCapsuleEnd = Matrix::Transpose(orientation) * (capsuleEnd - worldTransformB.GetPosition());
	// Closest point on capsule's centre line to the closest point of the OBB in local space
	Vector3 localClosestPoint = Vector::ClosestPointOnLineSegment(localCapsuleStart, localCapsuleEnd, localClosestPointOnBox);

	// Difference between the closest point in the capsule's centre line and the closest point in the OBB in local space
	Vector3 localToCapsule =  localClosestPoint - localClosestPointOnBox;

	// Distance between the capsule's origin and the closest point in the OBB
	float distance = Vector::Length(localToCapsule);

	// If no collision
	if (distance >= volumeA.GetRadius()) return false;

	// If there is a collision
	Vector3 collisionNormal = orientation * Vector::Normalise(localToCapsule);
	float penetration = volumeA.GetRadius() - distance;
	Vector3 localA = orientation * localClosestPointOnBox;
	Vector3 localB = -collisionNormal * volumeA.GetRadius();
	collisionInfo.AddContactPoint(localA, localB, collisionNormal, penetration);
	return true;
}

bool CollisionDetection::SphereCapsuleIntersection(
	const CapsuleVolume& volumeA, const Transform& worldTransformA,
	const SphereVolume& volumeB, const Transform& worldTransformB,
	CollisionInfo& collisionInfo) {

	Vector3 capsuleStart = worldTransformA.GetPosition() + (worldTransformA.GetOrientation() * Vector3(0, -volumeA.GetHalfHeight(), 0));
	Vector3 capsuleEnd = worldTransformA.GetPosition() + (worldTransformA.GetOrientation() * Vector3(0, volumeA.GetHalfHeight(), 0));

	Vector3 sphereCenter = worldTransformB.GetPosition();

	Vector3 closestPoint = Vector::ClosestPointOnLineSegment(capsuleStart, capsuleEnd, sphereCenter);
	Vector3 delta = sphereCenter - closestPoint;
	float distance = Vector::Length(delta);

	if (distance < volumeA.GetRadius() + volumeB.GetRadius()) {
		Vector3 normal = Vector::Normalise(delta);
		float penetration = (volumeA.GetRadius() + volumeB.GetRadius() - distance);

		collisionInfo.AddContactPoint(Vector3(), -normal * volumeB.GetRadius(), normal, penetration);
		return true;
	}
	return false;
}

Matrix4 GenerateInverseView(const Camera &c) {
	float pitch = c.GetPitch();
	float yaw	= c.GetYaw();
	Vector3 position = c.GetPosition();

	Matrix4 iview =
		Matrix::Translation(position) *
		Matrix::Rotation(-yaw, Vector3(0, -1, 0)) *
		Matrix::Rotation(-pitch, Vector3(-1, 0, 0));

	return iview;
}

Matrix4 GenerateInverseProjection(float aspect, float fov, float nearPlane, float farPlane) {
	float negDepth = nearPlane - farPlane;

	float invNegDepth = negDepth / (2 * (farPlane * nearPlane));

	Matrix4 m;

	float h = 1.0f / tan(fov*PI_OVER_360);

	m.array[0][0] = aspect / h;
	m.array[1][1] = tan(fov * PI_OVER_360);
	m.array[2][2] = 0.0f;

	m.array[2][3] = invNegDepth;//// +PI_OVER_360;
	m.array[3][2] = -1.0f;
	m.array[3][3] = (0.5f / nearPlane) + (0.5f / farPlane);

	return m;
}

Vector3 CollisionDetection::Unproject(const Vector3& screenPos, const PerspectiveCamera& cam) {
	Vector2i screenSize = Window::GetWindow()->GetScreenSize();

	float aspect = Window::GetWindow()->GetScreenAspect();
	float fov		= cam.GetFieldOfVision();
	float nearPlane = cam.GetNearPlane();
	float farPlane  = cam.GetFarPlane();

	Matrix4 invVP = GenerateInverseView(cam) * GenerateInverseProjection(aspect, fov, nearPlane, farPlane);
	Matrix4 proj  = cam.BuildProjectionMatrix(aspect);

	Vector4 clipSpace = Vector4(
		(screenPos.x / (float)screenSize.x) * 2.0f - 1.0f,
		(screenPos.y / (float)screenSize.y) * 2.0f - 1.0f,
		(screenPos.z),
		1.0f
	);

	Vector4 transformed = invVP * clipSpace;
	return Vector3(transformed.x / transformed.w, transformed.y / transformed.w, transformed.z / transformed.w);
}

Ray CollisionDetection::BuildRayFromMouse(const PerspectiveCamera& cam) {
	Vector2 screenMouse = Window::GetMouse()->GetAbsolutePosition();
	Vector2i screenSize	= Window::GetWindow()->GetScreenSize();

	Vector3 nearPos = Vector3(screenMouse.x,
		screenSize.y - screenMouse.y,
		-0.99999f
	);

	Vector3 farPos = Vector3(screenMouse.x,
		screenSize.y - screenMouse.y,
		0.99999f
	);

	Vector3 a = Unproject(nearPos, cam);
	Vector3 b = Unproject(farPos, cam);
	Vector3 c = b - a;

	c = Vector::Normalise(c);

	return Ray(cam.GetPosition(), c);
}

//http://bookofhook.com/mousepick.pdf
Matrix4 CollisionDetection::GenerateInverseProjection(float aspect, float fov, float nearPlane, float farPlane) {
	Matrix4 m;

	float t = tan(fov*PI_OVER_360);

	float neg_depth = nearPlane - farPlane;

	const float h = 1.0f / t;

	float c = (farPlane + nearPlane) / neg_depth;
	float e = -1.0f;
	float d = 2.0f*(nearPlane*farPlane) / neg_depth;

	m.array[0][0] = aspect / h;
	m.array[1][1] = tan(fov * PI_OVER_360);
	m.array[2][2] = 0.0f;

	m.array[2][3] = 1.0f / d;

	m.array[3][2] = 1.0f / e;
	m.array[3][3] = -c / (d * e);

	return m;
}

Matrix4 CollisionDetection::GenerateInverseView(const Camera &c) {
	float pitch = c.GetPitch();
	float yaw	= c.GetYaw();
	Vector3 position = c.GetPosition();

	Matrix4 iview =
		Matrix::Translation(position) *
		Matrix::Rotation(yaw, Vector3(0, 1, 0)) *
		Matrix::Rotation(pitch, Vector3(1, 0, 0));

	return iview;
}

Vector3	CollisionDetection::UnprojectScreenPosition(Vector3 position, float aspect, float fov, const PerspectiveCamera& c) {
	
	Matrix4 invVP = GenerateInverseView(c) * GenerateInverseProjection(aspect, fov, c.GetNearPlane(), c.GetFarPlane());
	Vector2i screenSize = Window::GetWindow()->GetScreenSize();

	Vector4 clipSpace = Vector4(
		(position.x / (float)screenSize.x) * 2.0f - 1.0f,
		(position.y / (float)screenSize.y) * 2.0f - 1.0f,
		(position.z) - 1.0f,
		1.0f
	);

	Vector4 transformed = invVP * clipSpace;
	return Vector3(transformed.x / transformed.w, transformed.y / transformed.w, transformed.z / transformed.w);
}

