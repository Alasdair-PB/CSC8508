//
// Contributors: Alasdair
//

#include "IComponent.h"
#include "GameObject.h"

using namespace NCL::CSC8508;

GameObject& IComponent::GetGameObject() {
	return this->gameObject;
}

bool IComponent::IsEnabled() const {
	return this->enabled;
}

void IComponent::SetEnabled(bool enabled) {
	this->enabled = enabled;
}