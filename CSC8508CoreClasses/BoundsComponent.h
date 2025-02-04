#pragma once
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

		BoundsComponent(GameObject& gameObject, CollisionVolume* collisionVolume);

		~BoundsComponent();

		/**
		 * Function invoked each frame.
		 * @param deltaTime Time since last frame
		 */
		void Update(float deltaTime) override;

		/**
		 * Function invoked each frame after Update.
		 * @param deltaTime Time since last frame
		 */
		void LateUpdate(float deltaTime) override;

		/**
		 * Function invoked when the component is enabled.
		 */
		void OnEnable() override;

		/**
		 * Function invoked when the component is disabled.
		 */
		void OnDisable() override;

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
