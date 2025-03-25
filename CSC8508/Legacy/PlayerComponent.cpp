#include "PlayerComponent.h"
using namespace NCL;
using namespace CSC8508;


PlayerComponent::PlayerComponent(GameObject& gameObject) :
	IComponent(gameObject),
	transform(gameObject.GetTransform()),
	isDashing(false),
	isGrounded(false),
	isJumping(false),
	onDashBinding(0),
	onJumpBinding(0),
	onItemInteractBinding(0),
	onInvScrollBinding(0)
{

}

PlayerComponent::~PlayerComponent() {

}
