#pragma once
#include "Transform.h"
#include "CollisionVolume.h"

using std::vector;

namespace NCL::CSC8508 {

	namespace Tags {
		enum Tag { Default, Player, Enemy, Kitten, CursorCast, Ground, Collect };
	}

	namespace Layers {
		enum LayerID { Default, Ignore_RayCast, UI, Player, Enemy, Ignore_Collisions };
	}

	class NetworkObject;
	class RenderObject;
	class PhysicsObject;

	class GameObject	{
	public:
		GameObject(const std::string& name = "");
		~GameObject();

		void SetBoundingVolume(CollisionVolume* vol) {
			boundingVolume = vol;
		}

		const CollisionVolume* GetBoundingVolume() const {
			return boundingVolume;
		}

		bool IsActive() const {
			return isActive;
		}

		Transform& GetTransform() {
			return transform;
		}

		RenderObject* GetRenderObject() const {
			return renderObject;
		}

		PhysicsObject* GetPhysicsObject() const {
			return physicsObject;
		}

		NetworkObject* GetNetworkObject() const {
			return networkObject;
		}

		void SetRenderObject(RenderObject* newObject) {
			renderObject = newObject;
		}

		void SetNetworkObject(NetworkObject* newObject) {
			networkObject = newObject;
		}

		void SetPhysicsObject(PhysicsObject* newObject) {
			physicsObject = newObject;
		}

		const std::string& GetName() const {
			return name;
		}

		virtual void OnCollisionBegin(GameObject* otherObject) {
			//std::cout << "OnCollisionBegin event occured!\n";
		}

		virtual void OnCollisionEnd(GameObject* otherObject) {
			//std::cout << "OnCollisionEnd event occured!\n";
		}

		bool GetBroadphaseAABB(Vector3&outsize) const;

		void UpdateBroadphaseAABB();

		void SetWorldID(int newID) {
			worldID = newID;
		}

		int	GetWorldID() const {
			return worldID;
		}	

		void SetLayerID(Layers::LayerID newID) { layerID = newID;}
		Layers::LayerID GetLayerID() const {return layerID; }
		void SetTag(Tags::Tag newTag) {  tag = newTag;}
		Tags::Tag GetTag() const { return tag;}

		void AddToIgnoredLayers(Layers::LayerID layerID) { ignoreLayers.push_back(layerID); }
		const std::vector<Layers::LayerID>& GetIgnoredLayers() const { return ignoreLayers; }

		float GetRestitution() { return restitution; }
		void SetRestitution(float newRestitution) { restitution = newRestitution;}


	protected:
		Transform			transform;

		CollisionVolume*	boundingVolume;
		PhysicsObject*		physicsObject;
		RenderObject*		renderObject;
		NetworkObject*		networkObject;

		bool isActive;
		int	worldID;
		float restitution = 0.2f; 

		Layers::LayerID	layerID;
		Tags::Tag	tag;
		std::string	name;

		Vector3 broadphaseAABB;
		vector<Layers::LayerID> ignoreLayers; // Made only for ignoring impluse resolution. Triggers will still activate
	};
}

