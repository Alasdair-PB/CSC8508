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

		void SetBoundingVolume(CollisionVolume* vol) { boundingVolume = vol;}

		const CollisionVolume* GetBoundingVolume() const { return boundingVolume;}
		void LoadVolume(bool isTrigger, VolumeType volumeType, Vector3 boundsSize);
		const PhysicsComponent* GetPhysicsComponent() const { return physicsComponent;}

		bool GetBroadphaseAABB(Vector3& outsize);
		void UpdateBroadphaseAABB();
		void SetPhysicsComponent(PhysicsComponent* physicsComponent) { this->physicsComponent = physicsComponent; }

		void AddToIgnoredLayers(Layers::LayerID layerID) { ignoreLayers.push_back(layerID); }
		const std::vector<Layers::LayerID>& GetIgnoredLayers() const { return ignoreLayers; }

		/// <summary>
		/// Get all types this IComponent may depend on when loading
		/// </summary>
		/// <returns>An unordered set of dependent IComponents</returns>
		std::unordered_set<std::type_index>& GetDependentTypes() const override {
			static std::unordered_set<std::type_index> types = { std::type_index(typeid(PhysicsComponent)) };
			return types;
		}

		/// <summary>
		/// IComponent Save data struct definition
		/// </summary>
		struct BoundsComponentDataStruct;

		/// <summary>
		/// Loads the PhysicsComponent save data into this PhysicsComponent
		/// </summary>
		/// <param name="assetPath">The loaded PhysicsComponent save data </param>
		/// <param name="allocationStart">The location this PhysicsComponent is saved in the asset file </param>
		virtual void Load(std::string assetPath, size_t allocationStart) override;
#
		/// <summary>
		/// Saves the PhysicsComponent data into the assetPath file.
		/// </summary>
		/// <param name="assetPath">The loaded PhysicsComponent save data </param>
		/// <param name="allocationStart">The location this PhysicsComponent is saved in the asset file </param>
		virtual size_t Save(std::string assetPath, size_t* allocationStart) override;

	protected:
		CollisionVolume* boundingVolume;
		PhysicsComponent* physicsComponent;
		Vector3 broadphaseAABB;
		vector<Layers::LayerID> ignoreLayers;

		Vector3 GetBoundsScale();
	};
}

#endif //BOUNDSCOMPONENT_H
