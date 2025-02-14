#include "CollisionDetection.h"
#include "CollisionVolume.h"
#include "AABBVolume.h"
#include "OBBVolume.h"
#include "SphereVolume.h"
#include "Window.h"
#include "Maths.h"
#include "Debug.h"

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

bool CollisionDetection::RayIntersection(const Ray& r,GameObject& object, RayCollision& collision) {
	bool hasCollided = false;

	const Transform& worldTransform = object.GetTransform();
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
			return false; // best intersection doesn ’t touch the box !
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

bool CollisionDetection::ObjectIntersection(GameObject* a, GameObject* b, CollisionInfo& collisionInfo) {
	const CollisionVolume* volA = a->GetBoundingVolume();
	const CollisionVolume* volB = b->GetBoundingVolume();

	if (!volA || !volB) {
		return false;
	}

	collisionInfo.a = a;
	collisionInfo.b = b;

	Transform& transformA = a->GetTransform();
	Transform& transformB = b->GetTransform();

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



struct OBB {
	Vector3 c; // OBB center point
	Vector3 u[3]; // Local x-, y-, and z-axes
	Vector3 e; // Positive halfwidth extents of OBB along each axis
};



void ClosestPtPointOBB(Vector3 p, OBB b, Vector3& q)
{
	Vector3 d = p - b.c;
	q = b.c;

	for (int i = 0; i < 3; i++) 
	{
		float dist = Vector::Dot(d, b.u[i]);
		if (dist > b.e[i]) dist = b.e[i];
		if (dist < -b.e[i]) dist = -b.e[i];
		q += b.u[i] * dist;
	}
}

float SqDistPointOBB(Vector3 p, OBB b)
{
	Vector3 v = p - b.c;
	float sqDist = 0.0f;
	for (int i = 0; i < 3; i++) {
		float d = Vector::Dot(v, b.u[i]), excess = 0.0f;
		if (d < -b.e[i])
			excess = d + b.e[i];
		else if (d > b.e[i])
			excess = d - b.e[i];
		sqDist += excess * excess;
	}
	return sqDist;
}


int TestOBBOBB(OBB& a, OBB& b)
{
	float ra, rb;
	Matrix3 R, AbsR;

	for (int i = 0; i < 3; i++)
	{
		Vector3 row;

		for (int j = 0; j < 3; j++) 
			row[j] = Vector::Dot(a.u[i], b.u[j]);
		R.SetRow(i, row);
	}

	Vector3 t = b.c - a.c;
	t = Vector3(
		Vector::Dot(t, a.u[0]),
		Vector::Dot(t, a.u[1]),
		Vector::Dot(t, a.u[2])
	);

	for (int i = 0; i < 3; i++) {
		Vector3 row;
		for (int j = 0; j < 3; j++) {
			row[j] = std::abs(R.GetRow(i)[j]) + FLT_EPSILON; // +EPSILON
		}
		AbsR.SetRow(i, row);
	}

	for (int i = 0; i < 3; i++) {
		ra = a.e[i];
		rb = b.e[0] * AbsR.GetRow(i)[0] + b.e[1] * AbsR.GetRow(i)[1] + b.e[2] * AbsR.GetRow(i)[2];
		if (std::abs(t[i]) > ra + rb) return 0;
	}

	for (int i = 0; i < 3; i++) {
		ra = a.e[0] * AbsR.GetRow(0)[i] + a.e[1] * AbsR.GetRow(1)[i] + a.e[2] * AbsR.GetRow(2)[i];
		rb = b.e[i];
		if (std::abs(t[0] * R.GetRow(0)[i] + t[1] * R.GetRow(1)[i] + t[2] * R.GetRow(2)[i]) > ra + rb) return 0;
	}

	ra = a.e[1] * AbsR.GetRow(2)[0] + a.e[2] * AbsR.GetRow(1)[0];
	rb = b.e[1] * AbsR.GetRow(0)[2] + b.e[2] * AbsR.GetRow(0)[1];
	if (std::abs(t[2] * R.GetRow(1)[0] - t[1] * R.GetRow(2)[0]) > ra + rb) return 0;

	ra = a.e[1] * AbsR.GetRow(2)[1] + a.e[2] * AbsR.GetRow(1)[1];
	rb = b.e[0] * AbsR.GetRow(0)[2] + b.e[2] * AbsR.GetRow(0)[0];
	if (std::abs(t[2] * R.GetRow(1)[1] - t[1] * R.GetRow(2)[1]) > ra + rb) return 0;

	ra = a.e[1] * AbsR.GetRow(2)[2] + a.e[2] * AbsR.GetRow(1)[2];
	rb = b.e[0] * AbsR.GetRow(0)[1] + b.e[1] * AbsR.GetRow(0)[0];
	if (std::abs(t[2] * R.GetRow(1)[2] - t[1] * R.GetRow(2)[2]) > ra + rb) return 0;

	ra = a.e[0] * AbsR.GetRow(2)[0] + a.e[2] * AbsR.GetRow(0)[0];
	rb = b.e[1] * AbsR.GetRow(1)[2] + b.e[2] * AbsR.GetRow(1)[1];
	if (std::abs(t[0] * R.GetRow(2)[0] - t[2] * R.GetRow(0)[0]) > ra + rb) return 0;

	ra = a.e[0] * AbsR.GetRow(2)[1] + a.e[2] * AbsR.GetRow(0)[1];
	rb = b.e[0] * AbsR.GetRow(1)[2] + b.e[2] * AbsR.GetRow(1)[0];
	if (std::abs(t[0] * R.GetRow(2)[1] - t[2] * R.GetRow(0)[1]) > ra + rb) return 0;

	ra = a.e[0] * AbsR.GetRow(2)[2] + a.e[2] * AbsR.GetRow(0)[2];
	rb = b.e[0] * AbsR.GetRow(1)[1] + b.e[1] * AbsR.GetRow(1)[0];
	if (std::abs(t[0] * R.GetRow(2)[2] - t[2] * R.GetRow(0)[2]) > ra + rb) return 0;

	ra = a.e[0] * AbsR.GetRow(1)[0] + a.e[1] * AbsR.GetRow(0)[0];
	rb = b.e[1] * AbsR.GetRow(2)[2] + b.e[2] * AbsR.GetRow(2)[1];
	if (std::abs(t[1] * R.GetRow(0)[0] - t[0] * R.GetRow(1)[0]) > ra + rb) return 0;

	ra = a.e[0] * AbsR.GetRow(1)[1] + a.e[1] * AbsR.GetRow(0)[1];
	rb = b.e[0] * AbsR.GetRow(2)[2] + b.e[2] * AbsR.GetRow(2)[0];
	if (std::abs(t[1] * R.GetRow(0)[1] - t[0] * R.GetRow(1)[1]) > ra + rb) return 0;

	ra = a.e[0] * AbsR.GetRow(1)[2] + a.e[1] * AbsR.GetRow(0)[2];
	rb = b.e[0] * AbsR.GetRow(2)[1] + b.e[1] * AbsR.GetRow(2)[0];
	if (std::abs(t[1] * R.GetRow(0)[2] - t[0] * R.GetRow(1)[2]) > ra + rb) return 0;

	return 1;
}


bool CollisionDetection::OBBIntersection(
	const OBBVolume& volumeA, const Transform& worldTransformA,
	const OBBVolume& volumeB, const Transform& worldTransformB,
	CollisionInfo& collisionInfo) {

	Matrix3 orientationA = Quaternion::RotationMatrix<Matrix3>(worldTransformA.GetOrientation());
	Matrix3 orientationB = Quaternion::RotationMatrix<Matrix3>(worldTransformB.GetOrientation());

	Vector3 positionA = worldTransformA.GetPosition();
	Vector3 positionB = worldTransformB.GetPosition();

	Vector3 halfSizeA = volumeA.GetHalfDimensions();
	Vector3 halfSizeB = volumeB.GetHalfDimensions();

	OBB obbA;
	obbA.c = positionA;
	obbA.u[0] = orientationA.GetRow(0);
	obbA.u[1] = orientationA.GetRow(1);
	obbA.u[2] = orientationA.GetRow(2);
	obbA.e = halfSizeA;

	OBB obbB;
	obbB.c = positionB;
	obbB.u[0] = orientationB.GetRow(0);
	obbB.u[1] = orientationB.GetRow(1);
	obbB.u[2] = orientationB.GetRow(2);
	obbB.e = halfSizeB;

	if (TestOBBOBB(obbA, obbB) == 0) {
		return false; 
	}

	Vector3 closestPointA, closestPointB;
	ClosestPtPointOBB(obbB.c, obbA, closestPointA);
	ClosestPtPointOBB(obbA.c, obbB, closestPointB);

	Vector3 collisionNormal = Vector::Normalise(closestPointB - closestPointA);
	float penetration = sqrt(SqDistPointOBB(closestPointB, obbA));

	Vector3 contactPointA = closestPointA + collisionNormal * penetration * 0.5f;
	Vector3 contactPointB = closestPointB - collisionNormal * penetration * 0.5f;


	Vector3 delta = positionB - positionA;

	if (Vector::Dot(collisionNormal, delta) < 0)
		collisionNormal = -collisionNormal;

	collisionInfo.AddContactPoint(contactPointA, contactPointB, collisionNormal, penetration);

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
	/*Vector3 capsuleStart = worldTransformA.GetPosition() + (worldTransformA.GetOrientation() * Vector3(0, -volumeA.GetHalfHeight(), 0));
	Vector3 capsuleEnd = worldTransformA.GetPosition() + (worldTransformA.GetOrientation() * Vector3(0, volumeA.GetHalfHeight(), 0));

	Vector3 boxMin = worldTransformB.GetPosition() - volumeB.GetHalfDimensions();
	Vector3 boxMax = worldTransformB.GetPosition() + volumeB.GetHalfDimensions();

	//Vector3 closestPoint = Vector::ClosestPointOnLineSegment(capsuleStart, capsuleEnd, boxMin, boxMax);
	Vector3 delta = closestPoint - boxMin;

	float distance = Vector::Length(delta);
	if (distance < volumeA.GetRadius()) {
		Vector3 normal = Vector::Normalise(delta);
		float penetration = (volumeA.GetRadius() - distance);

		collisionInfo.AddContactPoint(Vector3(), -normal * volumeA.GetRadius(), normal, penetration);
		return true;
	}*/
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

	Matrix3 orientation = Quaternion::RotationMatrix<Matrix3>(worldTransformB.GetOrientation());
	Vector3 boxSize = volumeB.GetHalfDimensions();
	Vector3 delta = worldTransformA.GetPosition() - worldTransformB.GetPosition();

	Vector3 localPoint = Matrix::Transpose(orientation) * delta;
	Vector3 negativeBoxSize = -boxSize;
	Vector3 closestPointOnBox = Vector::Clamp(localPoint, negativeBoxSize, boxSize);

	// As Capsule
	Vector3 capsuleStart = worldTransformA.GetPosition() + (worldTransformA.GetOrientation() * Vector3(0, -volumeA.GetHalfHeight(), 0));
	Vector3 capsuleEnd = worldTransformA.GetPosition() + (worldTransformA.GetOrientation() * Vector3(0, volumeA.GetHalfHeight(), 0));
	Vector3 closestPoint = Vector::ClosestPointOnLineSegment(capsuleStart, capsuleEnd, closestPointOnBox);
	Vector3 localToCapsule =   closestPoint - closestPointOnBox;

	// As Sphere
	Vector3 localToSphere = localPoint - closestPointOnBox;
	float distance = Vector::Length(localToSphere); 

	if (distance < volumeA.GetRadius())
	{
		//Vector3 collisionNormal = orientation * Vector::Normalise(localToCapsule);
		Vector3 collisionNormal = orientation * Vector::Normalise(localToSphere);

		float penetration = volumeA.GetRadius() - distance;

		Vector3 localA = orientation * closestPointOnBox;
		Vector3 localB = -collisionNormal * volumeA.GetRadius();
		collisionInfo.AddContactPoint(localA, localB, collisionNormal, penetration);
		return true;
	}
	return false;
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

