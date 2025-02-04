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
	class IComponent;
	class BoundsComponent;

	class GameObject	{
	public:
		GameObject(const std::string& name = "", bool isStatic = false);
		~GameObject();

		bool IsEnabled() const { return isEnabled;}
		bool SetEnabled(bool isEnabled) { this->isEnabled = isEnabled;  }

		bool IsStatic() const { return isStatic;}

		Transform& GetTransform() {return transform;}


		/**
		 * Function invoked after the object and components have been instantiated.
		 * @param deltaTime Time since last frame
		 */
		void InvokeOnAwake() { OnAwake(); }

		/**
		 * Function invoked each frame.
		 * @param deltaTime Time since last frame
		 */
		void InvokeUpdate(float deltaTime) { Update(deltaTime); }

		/**
		 * Function invoked each frame after Update.
		 * @param deltaTime Time since last frame
		 */
		void InvokeLateUpdate(float deltaTime) { LateUpdate(deltaTime); }

		/**
		 * Function invoked when the component is enabled.
		 */
		void InvokeOnEnable() { OnEnable(); }

		/**
		 * Function invoked when the component is disabled.
		 */
		void InvokeOnDisable() { OnDisable(); }


		RenderObject* GetRenderObject() const {
			return renderObject;
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

		const std::string& GetName() const {
			return name;
		}

		virtual void OnCollisionBegin(BoundsComponent* otherObject) {
			//std::cout << "OnCollisionBegin event occured!\n";
		}

		virtual void OnCollisionEnd(BoundsComponent* otherObject) {
			//std::cout << "OnCollisionEnd event occured!\n";
		}

		void SetWorldID(int newID) {
			worldID = newID;
		}

		int	GetWorldID() const {
			return worldID;
		}	

		template <typename T, typename... Args>
		requires std::is_base_of_v<IComponent, T>
		T* AddComponent(Args&&... args) {
			T* component = new T(*this, std::forward<Args>(args)...);
			components.push_back(component);
			return component;
		}

		template <typename T>
		requires std::is_base_of_v<IComponent, T>
		T* TryGetComponent() {
			for (IComponent* component : components) {
				if (std::strcmp(component->GetType(), typeid(T).name()) == 0) {
					return static_cast<T*>(component);
				}
			}
			return nullptr;
		}

		void AddChild(GameObject* child);
		GameObject* TryGetParent();
		void SetParent(GameObject* parent);
		bool HasParent();
		void UpdateComponents();
		bool HasTag(Tags::Tag tag);
		template <typename T> bool HasComponent(T type);


		void SetLayerID(Layers::LayerID newID) { layerID = newID;}
		Layers::LayerID GetLayerID() const {return layerID; }
		void SetTag(Tags::Tag newTag) {  tag = newTag;}
		Tags::Tag GetTag() const { return tag;}



	protected:
		virtual void OnAwake() {}
		virtual void Update(float deltaTime) {}
		virtual void LateUpdate(float deltaTime) {}
		virtual void OnEnable() {}
		virtual void OnDisable() {}

		Transform transform;
		RenderObject* renderObject;
		NetworkObject* networkObject;
		GameObject* parent;

		vector<IComponent*> components; 

		bool isEnabled;
		const bool isStatic;
		int	worldID;

		Layers::LayerID	layerID;
		Tags::Tag	tag; // Change to vector
		std::string	name;
	};
}

