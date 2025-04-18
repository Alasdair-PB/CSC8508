//
// Contributors: Alasdair
//

#ifndef ICOMPONENT_H
#define ICOMPONENT_H

#include "Transform.h"
#include "GameObject.h"
#include "UIElementsGroup.h"
#include <unordered_set>
#include <tuple>

using namespace NCL;
using namespace CSC8508;
using namespace UI;

namespace NCL::CSC8508 
{
	class IComponent : public ISerializable
	{
	public:

		IComponent(GameObject& gameObject) : gameObject(gameObject), enabled(true) {}
		virtual ~IComponent() = default;
		
		static const char* Name() { return "Base";}
		virtual const char* GetName() const { return Name();}

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
		GameObject& GetGameObject() const;

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
		virtual const char* GetType() const {
			return typeid(*this).name();
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
		/// Query for dependent types of this IComponent
		/// </summary>
		/// <returns>A unordered set of types this component is dependent on</returns>
		virtual std::unordered_set<std::type_index>& GetDependentTypes() const {
			static std::unordered_set<std::type_index> types = {};
			return types;
		}

		/// <summary>
		/// IComponent Save data struct definition
		/// </summary>
		struct ComponentDataStruct;

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

		/// <summary>
		/// Saves the IComponent data into the assetPath file. 
		/// </summary>
		/// <param name="assetPath">The loaded IComponent save data </param>
		/// <param name="allocationStart">The location this IComponent is saved in the asset file </param>
		virtual size_t Save(std::string assetPath, size_t* allocationStart) override;

		virtual void CopyComponent(GameObject* gameObject);
		virtual void PushIComponentElementsInspector(UIElementsGroup& elementsGroup, float scale);

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
