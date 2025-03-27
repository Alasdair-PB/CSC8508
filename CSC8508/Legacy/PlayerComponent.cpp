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
	onInvScrollBinding(0),
	pickUpDelay(0.3f),
	timeSinceLastPickUp(0){}

PlayerComponent::~PlayerComponent(){}

void PlayerComponent::SetBindingJump(uint32_t j, StaminaComponent* component) {
    onJumpBinding = j;
    if (component) component->SetStaminaAction(j, jumpStamCost);
}

void PlayerComponent::SetBindingDash(uint32_t d, StaminaComponent* component) {
    onDashBinding = d;
    if (component) component->SetStaminaAction(d, dashTickStam);
}

void PlayerComponent::SetBindingInteract(uint32_t p) {
    onItemInteractBinding = p;
}

void PlayerComponent::SetBindingScrollInventory(uint32_t p) {
    onInvScrollBinding = p;
}

void PlayerComponent::OnEvent(InputButtonEvent* buttonEvent) {
    if (&buttonEvent->owner != &GetGameObject()) { return; }
    inputStack.push(buttonEvent->buttonId);
}

void PlayerComponent::OnEvent(CollisionEvent* collisionEvent)
{
    bool hasTag = (collisionEvent->object1.HasTag(Tags::Ground) && &collisionEvent->object2 == &GetGameObject()) ||
        (collisionEvent->object2.HasTag(Tags::Ground) && &collisionEvent->object1 == &GetGameObject());
    if (hasTag) {
        isGrounded = true;
    }
}

/**
 * Function invoked each frame after Update.
 * @param deltaTime Time since last frame
 */
void PlayerComponent::OnAwake()
{
    physicsComponent = GetGameObject().TryGetComponent<PhysicsComponent>();
    inputComponent = GetGameObject().TryGetComponent<InputComponent>();
    staminaComponent = GetGameObject().TryGetComponent<StaminaComponent>();
    inventoryComponent = GetGameObject().TryGetComponent<InventoryManagerComponent>();
    sightComponent = GetGameObject().TryGetComponent<SightComponent>();

    EventManager::RegisterListener<InputButtonEvent>(this);
    EventManager::RegisterListener<CollisionEvent>(this);

    if (physicsComponent)
        physicsObj = physicsComponent->GetPhysicsObject();
}

void PlayerComponent::OnDashInput() {
    if (staminaComponent->PerformActionIfAble(onDashBinding))
        isDashing = true;
}

void PlayerComponent::SetLinearVelocity(float jumpDuration) {
    Vector3 velocity = physicsObj->GetLinearVelocity();
    physicsObj->SetLinearVelocity(Vector3(velocity.x, jumpForce * (1 + (0.2f - jumpDuration)), velocity.z));
}

void PlayerComponent::OnJumpInput() {
    if (isJumping || !isGrounded) return;
    if (staminaComponent->PerformActionIfAble(onJumpBinding)) {
        isJumping = true;
        jumpDuration = 0;
        SetLinearVelocity(0);
    }
}

void PlayerComponent::OnJump(float deltaTime) {
    if (!isJumping) return;
    jumpDuration += deltaTime;
    if (jumpDuration < 0.2f)
        SetLinearVelocity(jumpDuration);
    else {
        if (physicsObj->GetLinearVelocity().y <= 0.0f)
            physicsObj->AddForce(Vector3(0, -1, 0) * downwardsVelocityMod);
        if (isGrounded) { isJumping = false; }
    }
}

bool PlayerComponent::DropItem() {
    if (inventoryComponent->ItemInHand()) {
        inventoryComponent->DropItem();
        timeSinceLastPickUp = 0;
        return true;
    }
    return false;
}

void PlayerComponent::PickUpItem(ItemComponent* item) {
    if (item) {
        inventoryComponent->PushItemToInventory(item);
        timeSinceLastPickUp = 0;
    }
}

Vector3 PlayerComponent::GetForwardsDirection() {
    return transform.GetOrientation() * Vector3(0, 0, 1);
}

void PlayerComponent::TryPickUp() {
    ItemComponent* item = sightComponent->CanSeeComponent<ItemComponent>(
        visibleRange, visionHeight, GetForwardsDirection());
    PickUpItem(item);
}

void PlayerComponent::OnItemInteract() {
    if (!inventoryComponent) return;
    if (timeSinceLastPickUp < pickUpDelay) return;
    if (DropItem()) return;
    TryPickUp();
}

void PlayerComponent::OnPlayerMove() {
    isMoving = false;
    if (inputComponent->GetNamedAxis("Forward") == 0 && inputComponent->GetNamedAxis("Sidestep") == 0)
        return;
    isMoving = true;
    Vector3 dir;
    Matrix3 yawRotation = inputComponent->GetMouseGameWorldYawMatrix();

    dir += yawRotation * Vector3(0, 0, -inputComponent->GetNamedAxis("Forward"));
    dir += yawRotation * Vector3(inputComponent->GetNamedAxis("Sidestep"), 0, 0);

    Matrix3 offsetRotation = Matrix::RotationMatrix3x3(0.0f, Vector3(0, 1, 0));
    dir = offsetRotation * dir;
    physicsObj->AddForce(dir * speed * (isDashing ? dashMultiplier : 1.0f));
    physicsObj->RotateTowardsVelocity();
}

void PlayerComponent::CheckInputStack() {
    while (!inputStack.empty()) {
        if (inputStack.top() == onDashBinding)
            OnDashInput();
        else if (inputStack.top() == onJumpBinding)
            OnJumpInput();
        else if (inputStack.top() == onItemInteractBinding)
            OnItemInteract();
        inputStack.pop();
    }
}

void PlayerComponent::UpdateStates(float deltaTime) {
    isDashing = false;
    isGrounded = !isJumping ? true : false;
    timeSinceLastPickUp += deltaTime;
}

void PlayerComponent::Update(float deltaTime)
{
    if (physicsObj == nullptr || physicsComponent == nullptr || inputComponent == nullptr || staminaComponent == nullptr)
        return;
    OnPlayerMove();
    CheckInputStack();
    OnJump(deltaTime);
    UpdateStates(deltaTime);
}