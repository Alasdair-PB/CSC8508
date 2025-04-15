#include "AnimationComponent.h"

namespace NCL {
	namespace CSC8508 {
		
		
		
		
		class PlayerAnimation : public AnimationComponent {
		public:
			PlayerAnimation(GameObject& gameObject) : AnimationComponent(gameObject) {



			}
			~PlayerAnimation();

			virtual std::unordered_set<std::type_index>& GetDerivedTypes() const override {
				static std::unordered_set<std::type_index> derivedTypes = {
					typeid(AnimationComponent),
					typeid(IStateComponent),
					typeid(IComponent)
				};
				return derivedTypes;
			}

		protected:

		};
	}
}