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
	onItemPickUpBinding(0)
{

}

PlayerComponent::~PlayerComponent() {

}
