#pragma once

#include "Camera.h"

#include "Transform.h"
#include "GameObject.h"
#include "BoundsComponent.h"

#include "AABBVolume.h"
#include "OBBVolume.h"
#include "SphereVolume.h"
#include "CapsuleVolume.h"
#include "Ray.h"

using NCL::Camera;
using namespace NCL::Maths;
using namespace NCL::CSC8508;
namespace NCL {
	class CollisionDetection
	{
	public:
		struct ContactPoint {
			Vector3 localA;
			Vector3 localB;
			Vector3 normal;
			float penetration;

			ContactPoint() : localA(Vector3()), 
				localB(Vector3()),
				normal(Vector3()) ,
				penetration(0.0f)
			{}
		};

		struct CollisionInfo {
			BoundsComponent* a;
			BoundsComponent* b;
			ContactPoint point;

			int framesLeft;
			CollisionInfo() : framesLeft(0), a(nullptr), b(nullptr) {}

			void AddContactPoint(const Vector3& localA, const Vector3& localB, const Vector3& normal, float p) {
				point.localA		= localA;
				point.localB		= localB;
				point.normal		= normal;
				point.penetration	= p;
			}

			bool operator < (const CollisionInfo& other) const {
				size_t otherHash = (size_t)other.a + ((size_t)other.b << 32);
				size_t thisHash  = (size_t)a + ((size_t)b << 32);

				if (thisHash < otherHash) {
					return true;
				}
				return false;
			}

			bool operator ==(const CollisionInfo& other) const {
				if (other.a == a && other.b == b) {
					return true;
				}
				return false;
			}
		};

		static bool AABBCapsuleIntersection(
			const CapsuleVolume& volumeA, const Transform& worldTransformA,
			const AABBVolume& volumeB, const Transform& worldTransformB, CollisionInfo& collisionInfo);

		static bool OBBCapsuleIntersection(
			const CapsuleVolume& volumeA, const Transform& worldTransformA,
			const OBBVolume& volumeB, const Transform& worldTransformB, CollisionInfo& collisionInfo);

		static bool SphereCapsuleIntersection(
			const CapsuleVolume& volumeA, const Transform& worldTransformA,
			const SphereVolume& volumeB, const Transform& worldTransformB, CollisionInfo& collisionInfo);

		//TODO ADD THIS PROPERLY
		static bool RayBoxIntersection(const Ray&r, const Vector3& boxPos, const Vector3& boxSize, RayCollision& collision);

		static Ray BuildRayFromMouse(const PerspectiveCamera& c);

		static bool RayIntersection(const Ray&r, BoundsComponent& object, RayCollision &collisions);


		static bool RayAABBIntersection(const Ray&r, const Transform& worldTransform, const AABBVolume&	volume, RayCollision& collision);
		static bool RayOBBIntersection(const Ray&r, const Transform& worldTransform, const OBBVolume&	volume, RayCollision& collision);
		static bool RaySphereIntersection(const Ray&r, const Transform& worldTransform, const SphereVolume& volume, RayCollision& collision);
		static bool RayCapsuleIntersection(const Ray& r, const Transform& worldTransform, const CapsuleVolume& volume, RayCollision& collision);


		static bool RayPlaneIntersection(const Ray&r, const Plane&p, RayCollision& collisions);

		static bool	AABBTest(const Vector3& posA, const Vector3& posB, const Vector3& halfSizeA, const Vector3& halfSizeB);

		static bool ObjectIntersection(GameObject* gameObjectA, GameObject* gameObjectB, CollisionInfo& collisionInfo);

		static bool ObjectIntersection(BoundsComponent* a, BoundsComponent* b, CollisionInfo& collisionInfo);


		static bool AABBIntersection(	const AABBVolume& volumeA, const Transform& worldTransformA,
										const AABBVolume& volumeB, const Transform& worldTransformB, CollisionInfo& collisionInfo);

		static bool SphereIntersection(	const SphereVolume& volumeA, const Transform& worldTransformA,
										const SphereVolume& volumeB, const Transform& worldTransformB, CollisionInfo& collisionInfo);

		static bool AABBSphereIntersection(	const AABBVolume& volumeA	 , const Transform& worldTransformA,
										const SphereVolume& volumeB, const Transform& worldTransformB, CollisionInfo& collisionInfo);

		static bool OBBIntersection(	const OBBVolume& volumeA, const Transform& worldTransformA,
										const OBBVolume& volumeB, const Transform& worldTransformB, CollisionInfo& collisionInfo);


		static bool OBBSphereIntersection(const OBBVolume& volumeA, const Transform& worldTransformA,
			const SphereVolume& volumeB, const Transform& worldTransformB, CollisionInfo& collisionInfo);


		static Vector3 Unproject(const Vector3& screenPos, const PerspectiveCamera& cam);

		static Vector3		UnprojectScreenPosition(Vector3 position, float aspect, float fov, const PerspectiveCamera&c);
		static Matrix4		GenerateInverseProjection(float aspect, float fov, float nearPlane, float farPlane);
		static Matrix4		GenerateInverseView(const Camera &c);

	protected:

	private:
		CollisionDetection()	{}
		~CollisionDetection()	{}
	};
}

