//
// Contributors: Alasdair
//

#ifndef ICOMPONENT_H
#define ICOMPONENT_H

#include "Transform.h"
#include "GameObject.h"
#include <unordered_set>

namespace NCL::CSC8508 
{
	class IComponent : public ISerializable
	{
	public:

		IComponent(GameObject& gameObject) : gameObject(gameObject), enabled(true) {}

		virtual ~IComponent() = default;

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

		/**
		 * Function invoked when the component is enabled.
		 */
		void InvokeOnEnable() { OnEnable(); }

		/**
		 * Function invoked when the component is disabled.
		 */
		void InvokeOnDisable() { OnDisable(); }

		/**
		* Function gets the GameObject this component is attatched to.
		* @return the GameObject this component is attatched to.
		*/
		GameObject& GetGameObject();

		/**
		* Function Gets the enabled state of the component.
		* @return the enabled state
		*/
		bool IsEnabled() const;

		/**
		* Function sets the enabled state of the component.
		* @param the new enabled state
		*/
		void SetEnabled(bool enabled);

		/**
		* Function gets the component type
		* @return the component type
		*/
		virtual std::string GetType() const {
			return SaveManager::Demangle(typeid(this).name());
		}

		/// <summary>
		/// Query for derived types of this IComponet. 
		/// </summary>
		/// <returns>A set of type_indexes of IComponents that are marked as derived from this IComponent</returns>
		virtual std::unordered_set<std::type_index>& GetDerivedTypes() const {
			static std::unordered_set<std::type_index> types = { std::type_index(typeid(IComponent)) };
			return types;
		}

		/// <summary>
		/// Query if this IComponent is derived from a type matching type_info
		/// </summary>
		/// <param name="typeInfo">The type info of the queried type</param>
		/// <returns>True if this IComponent derives from type_info otherwise returns false</returns>
		bool IsDerived(const std::type_info& typeInfo) const {
			return GetDerivedTypes().count(std::type_index(typeInfo)) > 0;
		}
		/// <summary>
		/// Loads the IComponent save data into this IComponent
		/// </summary>
		/// <param name="assetPath">The loaded IComponent save data </param>
		/// <param name="allocationStart">The location this IComponent is saved in the asset file </param>
		virtual void Load(std::string assetPath, size_t allocationStart) override;
#
		/// <summary>
		/// Saves the IComponent data into the assetPath file. 
		/// </summary>
		/// <param name="assetPath">The loaded IComponent save data </param>
		/// <param name="allocationStart">The location this IComponent is saved in the asset file </param>
		virtual size_t Save(std::string assetPath, size_t* allocationStart) override;


		/// <summary>
		/// IComponent Save data struct definition
		/// </summary>
		struct ComponentDataStruct;

	protected:
		virtual void OnAwake() {}
		virtual void Update(float deltaTime) {}
		virtual void EarlyUpdate(float deltaTime) {}
		virtual void OnEnable() {}
		virtual void OnDisable() {}
	private:
		GameObject& gameObject;
		bool enabled;

	};
}

#endif //ICOMPONENT_H
