//
// Contributors: Alasdair
//

#ifndef PHYSICSCOMPONENT_H
#define PHYSICSCOMPONENT_H

#include "Transform.h"
#include "IComponent.h"
#include "CollisionVolume.h"

using std::vector;

namespace NCL::CSC8508
{
	class PhysicsObject;
	 
	class PhysicsComponent : public IComponent
	{
	public:

		PhysicsComponent(GameObject& gameObject);
		~PhysicsComponent();

		enum InitType { None, Sphere, Cube };

		PhysicsObject* GetPhysicsObject() const {
			return physicsObject;
		}

		void SetPhysicsObject(PhysicsObject* newObject) {
			physicsObject = newObject;
		}

		void SetInitType(InitType type) {
			initiType = type;
		}

		/// <summary>
		/// IComponent Save data struct definition
		/// </summary>
		struct PhysicsComponentDataStruct;

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
		PhysicsObject* physicsObject;
		InitType initiType;
	};
}

#endif //PHYSICSCOMPONENT_H
