#pragma once
#include <random>
#include "Ray.h"
#include "INetworkComponent.h"
#include "CollisionDetection.h"
#include "QuadTree.h"

namespace NCL {
		class Camera;
		using Maths::Ray;
	namespace CSC8508 {
		class GameObject;
		class PhysicsComponent;
		class BoundsComponent;
		class Constraint;

		typedef std::function<void(GameObject*)> GameObjectFunc;	
		typedef std::vector<GameObject*>::const_iterator GameObjectIterator;


		// To remove all below
		typedef std::function<void(PhysicsComponent*)> PhysicsComponentFunc;
		typedef std::function<void(IComponent*)> IComponentFunc;

		typedef std::vector<PhysicsComponent*>::const_iterator PhysicsIterator;
		typedef std::vector<BoundsComponent*>::const_iterator BoundsIterator;
		typedef std::vector<IComponent*>::const_iterator ICompIterator;

		class GameWorld : ISerializable	{
		public:
			GameWorld();
			~GameWorld();

			void Clear();
			void ClearAndErase();

			void AddGameObject(GameObject* o);
			void RemoveGameObject(GameObject* o, bool andDelete = false);

			void AddConstraint(Constraint* c);
			void RemoveConstraint(Constraint* c, bool andDelete = false);

			PerspectiveCamera& GetMainCamera()  {
				return mainCamera;
			}
			void ShuffleWorldConstraints();

			void ShuffleConstraints(bool state) {
				shuffleConstraints = state;
			}

			void ShuffleObjects(bool state) {
				shuffleObjects = state;
			}
			void SetPausedWorld(bool state) {}

			bool Raycast(Ray& r, RayCollision& closestCollision, bool closestObject, BoundsComponent* ignoreThis = nullptr, vector<Layers::LayerID>* ignoreLayers = nullptr) const;

			virtual void UpdateWorld(float dt);

			void OperateOnContents(GameObjectFunc f);

			void GetObjectIterators(
				GameObjectIterator& first,
				GameObjectIterator& last) const;

			void GetPhysicsIterators(
				PhysicsIterator& first,
				PhysicsIterator& last) const;

			void GetBoundsIterators(
				BoundsIterator& first,
				BoundsIterator& last) const;

			void GetConstraintIterators(
				std::vector<Constraint*>::const_iterator& first,
				std::vector<Constraint*>::const_iterator& last) const;

			int GetWorldStateID() const {
				return worldStateCounter;
			}

			int GetGameObjectCount() const { return gameObjects.size(); }
			struct WorldSaveData;

			size_t Save(std::string assetPath, size_t* allocationStart = 0);
			void  Load(std::string assetPath, size_t allocationStart = 0);
			void LoadCameraInfo(float nearPlane, float farPlane, float pitch, float yaw, Vector3 position);

		protected:
			std::vector<PhysicsComponent*> physicsComponents;
			std::vector<BoundsComponent*> boundsComponents;

			std::vector<GameObject*> gameObjects;
			std::vector<IComponent*> components;

			std::vector<Constraint*> constraints;

			PerspectiveCamera mainCamera;

			bool shuffleConstraints;
			bool shuffleObjects;
			int worldIDCounter;
			int worldStateCounter;
		};
	}
}

