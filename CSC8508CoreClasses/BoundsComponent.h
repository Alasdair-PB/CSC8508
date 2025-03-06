//
// Contributors: Alasdair
//

#ifndef BOUNDSCOMPONENT_H
#define BOUNDSCOMPONENT_H

#include "Transform.h"
#include "IComponent.h"
#include "CollisionVolume.h"
#include "PhysicsComponent.h"
#include "GameObject.h" // Just for layers namespace

using std::vector;

namespace NCL::CSC8508
{
	class BoundsComponent : public IComponent
	{
	public:

		BoundsComponent(GameObject& gameObject, CollisionVolume* collisionVolume, PhysicsComponent* physicsComponent = nullptr);

		~BoundsComponent();

		void SetBoundingVolume(CollisionVolume* vol) {
			boundingVolume = vol;
		}

		const CollisionVolume* GetBoundingVolume() const {
			return boundingVolume;
		}


		const PhysicsComponent* GetPhysicsComponent() const {
			return physicsComponent;
		}

		bool GetBroadphaseAABB(Vector3& outsize) const;

		void UpdateBroadphaseAABB();

		void AddToIgnoredLayers(Layers::LayerID layerID) { ignoreLayers.push_back(layerID); }
		const std::vector<Layers::LayerID>& GetIgnoredLayers() const { return ignoreLayers; }

	protected:
		CollisionVolume* boundingVolume;
		PhysicsComponent* physicsComponent;
		Vector3 broadphaseAABB;
		vector<Layers::LayerID> ignoreLayers;
	};
}

#endif //BOUNDSCOMPONENT_H
