//
// Contributors: Alasdair
//

#pragma once
#include "Transform.h"
#include "ComponentManager.h"
#include "../SerializedSave/ISerializable.h"
#include "../Event/Event.h"
#include <vector>

using std::vector;

namespace NCL::CSC8508 {

	namespace Tags {
		enum Tag { Default, Player, Enemy, Kitten, CursorCast, Ground, Collect };
	}

	namespace Layers {
		enum LayerID { Default, Ignore_RayCast, UI, Player, Enemy, Ignore_Collisions };
	}
	class IComponent;
	class NetworkObject;
	class RenderObject;
	class BoundsComponent;

	class GameObject : ISerializable {
	
	public:
		GameObject(bool isStatic = false);
		~GameObject();

		/// <summary>
		/// Query if this GameObject is enabled
		/// </summary>
		/// <returns>True if this GameObject is enabled otherwise returns false</returns>
		bool IsEnabled() const { return isEnabled;}

		/// <summary>
		/// Sets the enabled state of this GameObject
		/// </summary>
		/// <param name="isEnabled">The new enabled state of this GameObject</param>
		/// <returns></returns>
		void SetEnabled(bool isEnabled) { this->isEnabled = isEnabled;  }

		/// <summary>
		/// Query if this GameObject is static
		/// </summary>
		/// <returns>True if this GameObject is static otherwise returns false</returns>
		bool IsStatic() const { return isStatic;}

		/// <summary>
		/// Get a pointer to the Transform attatched to this GameObject
		/// </summary>
		/// <returns>A pointer to the Transform attatched to this GameObject</returns>
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
		void InvokeEarlyUpdate(float deltaTime) { EarlyUpdate(deltaTime); }

		/* Function invoked when the component is enabled. */
		void InvokeOnEnable() { OnEnable(); }

		/* Function invoked when the component is disabled. */
		void InvokeOnDisable() { OnDisable(); }

		/// <summary>
		/// Returns the render object of this GameObject
		/// </summary>
		/// <returns>The render object of this GameObject</returns>
		RenderObject* GetRenderObject() const { return renderObject; }

		/// <summary>
		/// Sets the RenderObject of this GameObject
		/// </summary>
		/// <param name="newObject">The new RenderObject</param>
		void SetRenderObject(RenderObject* newObject) {
			renderObject = newObject;
		}

		/// <summary>
		/// Calls a collision Event when a Collision begins against this GameObjects Bounds Component
		/// </summary>
		/// <param name="otherObject">The Colliding other BoundsComponent</param>
		virtual void OnCollisionBegin(BoundsComponent* otherObject) {
			//std::cout << "OnCollisionBegin event occured!\n";
		}

		/// <summary>
		/// Calls a collision Event when a Collision ends against this GameObjects Bounds Component
		/// </summary>
		/// <param name="otherObject">The Colliding other BoundsComponent</param>
		virtual void OnCollisionEnd(BoundsComponent* otherObject) {
			//std::cout << "OnCollisionEnd event occured!\n";
		}

		/// <summary>
		/// Sets the unique world identifier of this GameObject
		/// </summary>
		/// <param name="newID">The new world id of the Gameobject</param>
		void SetWorldID(int newID) { worldID = newID; }

		/// <summary>
		/// Returns the unique world identifier of this GameObject
		/// </summary>
		/// <returns>The world id of the Gameobject</returns>
		int	GetWorldID() const { return worldID;}

		/// <summary>
		/// Get all Components attatched to this GameObject
		/// </summary>
		/// <returns>A vector of all IComponents attatched to this GameObject</returns>
		vector<IComponent*> GetAllComponents() const { return components; }

		/// <summary>
		/// Adds component of type T to the component
		/// </summary>
		/// <typeparam name="T">The component type</typeparam>
		/// <typeparam name="...Args">The argument Types passed for Component of type T's constructor</typeparam>
		/// <param name="...args">The arguments passed for Component of type T's constructor</param>
		/// <returns></returns>
		template <typename T, typename... Args>
			requires std::is_base_of_v<IComponent, T>
		T* AddComponent(Args&&... args) {
			T* component = ComponentManager::AddComponent<T>(*this, std::forward<Args>(args)...);
			components.push_back(component);
			return component;
		}

		/// <summary>
		/// Returns a pointer to a attatched GameObject if the component
		/// </summary>
		/// <typeparam name="T"></typeparam>
		/// <returns></returns>
		template <typename T>
			requires std::is_base_of_v<IComponent, T>
		T* TryGetComponent() {
			for (IComponent* component : components) {
				if (T* casted = dynamic_cast<T*>(component))
					return casted;
			}
			return nullptr;
		}

		/// <summary>
		/// GameObject Save data struct definition
		/// </summary>
		struct GameObjDataStruct; 

		/// <summary>
		/// Loads the gameobject save data into the GameObject and its components. 
		/// </summary>
		/// <param name="assetPath">The loaded GaemObject save data </param>
		/// <param name="allocationStart">The location this object is saved in the asset file </param>
		void Load(std::string assetPath, size_t allocationStart = 0) override;

		/// <summary>
		/// Saves the gameobject and its components data into the assetPath file. 
		/// </summary>
		/// <param name="assetPath">The loaded GaemObject save data </param>
		/// <param name="allocationStart">The location this object is saved in the asset file </param>
		size_t Save(std::string assetPath, size_t* = nullptr) override;

		/// <summary>
		/// Loads the save data as new components
		/// </summary>
		/// <param name="loadedSaveData">The loaded GaemObject save data </param>
		/// <param name="assetPath">The path to the asset file</param>
		void LoadClean(GameObjDataStruct& loadedSaveData, std::string assetPath);

		/// <summary>
		/// Loads the save data into exisiting components
		/// </summary>
		/// <param name="loadedSaveData">The loaded GaemObject save data </param>
		/// <param name="assetPath">The path to the asset file</param>
		void LoadInto(GameObjDataStruct& loadedSaveData, std::string assetPath);

		/// <summary>
		/// Loads GameObject specific Data into this GameObject
		/// </summary>
		/// <param name="loadedSaveData">The loaded data used to set this GameObject's properties to the loaded data</param>
		void LoadGameObjectInstanceData(GameObjDataStruct loadedSaveData);

		/// <summary>
		/// Add a child GameObject to this GameObject
		/// </summary>
		/// <param name="child">The new child GameObject</param>
		void AddChild(GameObject* child);

		/// <summary>
		/// Return the Parent Gameobject of this GameObject otherwise return nullptr
		/// </summary>
		/// <returns>The parent gameobject if one exists otherwise returnn nullptr</returns>
		GameObject* TryGetParent();

		/// <summary>
		/// Sets the Parent of this GameObject
		/// </summary>
		/// <param name="parent">The new parent of this GameObject</param>
		void SetParent(GameObject* parent);

		/// <summary>
		/// Checks if this GameObject has a parent otherwise false
		/// </summary>
		/// <returns>true if this GameObject has a parent otherwise false</returns>
		bool HasParent();
		
		/// <summary>
		/// Check if this GameObject has a tag that matches the param tag
		/// </summary>
		/// <param name="tag">The queried tag</param>
		/// <returns>True if the tag is found otherwise returns false</returns>
		bool HasTag(Tags::Tag tag);

		/// <summary>
		/// Query if this GameObject has a component of type T attatched
		/// </summary>
		/// <typeparam name="T">The type of the queried Component</typeparam>
		/// <param name="type">The queried Component</param>
		/// <returns>True if a component of type T is attatched to this GameObject otherwise returns false</returns>
		template <typename T> bool HasComponent(T type);

		void SetLayerID(Layers::LayerID newID) { layerID = newID;}
		Layers::LayerID GetLayerID() const {return layerID; }
		void SetTag(Tags::Tag newTag) {  tag = newTag;}
		Tags::Tag GetTag() const { return tag;}

	protected:
		virtual void OnAwake() {}
		virtual void Update(float deltaTime) {}
		virtual void EarlyUpdate(float deltaTime) {}
		virtual void OnEnable() {}
		virtual void OnDisable() {}

		bool isEnabled;
		const bool isStatic;
		int worldID;

		Transform transform;
		RenderObject* renderObject;
		GameObject* parent;
		vector<IComponent*> components; 

		Layers::LayerID	layerID;
		Tags::Tag tag; 
	};

	/// <summary>
	/// An Event class that defines the data sent to the ComponentAssemblyDefiner where the 
	/// ComponentAssemblyDefiner defines how components are added to this GameObject from a type mapped id
	/// </summary>
	class AddComponentEvent : public CancellableEvent {
	public:
		AddComponentEvent(GameObject& gameObject, size_t entry);
		GameObject& GetGameObject();
		size_t GetEntry();
	protected:
		GameObject& gameObject;
		size_t entry;
	};

}

