#pragma once
#include "GameObject.h"

namespace NCL {
	namespace CSC8508 {

		//this but empty functions, and needs gameobject& 
		class  IState {
		public:
			IState() {}

			virtual void Update(float dt, GameObject& gameObject) {
				
			}
		protected:

		};
	}
}