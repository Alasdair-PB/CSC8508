#include "PlayerComponent.h"
#include "../Event/EventManager.h"
#include "../Event/Event.h"

using namespace NCL;
using namespace CSC8508;

PlayerComponent::PlayerComponent(GameObject& gameObject) :
	IComponent(gameObject),
	transform(gameObject.GetTransform()),
	isDashing(false),
	isGrounded(false),
	isJumping(false),
    inDropZone(false),
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

void PlayerComponent::SetBindingPause(uint32_t p) {
	onPauseBinding = p;
}

void PlayerComponent::SetBindingDebug(uint32_t d) {
	onDebugBinding = d;
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

bool PlayerComponent::CheckTag(Tag tag, CollisionEvent* collisionEvent) {
    return (collisionEvent->object1.HasTag(tag) && &collisionEvent->object2 == &GetGameObject()) ||
        (collisionEvent->object2.HasTag(tag) && &collisionEvent->object1 == &GetGameObject());
}

void PlayerComponent::OnEvent(CollisionEvent* collisionEvent)
{
    if (CheckTag(Tag::Ground, collisionEvent))
        collidedTags.push(Tag::Ground);
    if (CheckTag(Tag::DropZone, collisionEvent))
        collidedTags.push(Tag::DropZone);
    if (CheckTag(Tag::DepositZone, collisionEvent))
        collidedTags.push(Tag::DepositZone);
	if (CheckTag(Tag::Exit, collisionEvent))
		collidedTags.push(Tag::Exit);
}

void PlayerComponent::OnEvent(DeathEvent* deathEvent) {
    if (&deathEvent->GetGameObject() == &GetGameObject()) {
        std::cout << "dead" << "\n";
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
    EventManager::RegisterListener<DeathEvent>(this);

    if (physicsComponent)
        physicsObj = physicsComponent->GetPhysicsObject();
}

void PlayerComponent::OnDashInput() {
    if (staminaComponent->PerformActionIfAble(onDashBinding))
        isDashing = true;
}

void PlayerComponent::SetLinearVelocity(float jumpDuration) {
    Vector3 velocity = physicsObj->GetLinearVelocity();
    GetWeightModifier();
    physicsObj->SetLinearVelocity(Vector3(velocity.x, jumpForce * weightModifier * (1 + (0.2f - jumpDuration)), velocity.z));
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
        if (isGrounded) { isJumping = false; }
    }
}

void PlayerComponent::OnPauseInput() {
		PauseEvent* e = new PauseEvent();
		EventManager::Call<PauseEvent>(e);
}

void PlayerComponent::OnDebugInput() {
	DebugEvent* e = new DebugEvent();
	EventManager::Call<DebugEvent>(e);
}

bool PlayerComponent::DropItemToFloor() {
    inventoryComponent->DropItem();
    timeSinceLastPickUp = 0;
    return true;
}

bool PlayerComponent::DropItemToDropZone() {
    inventoryComponent->SellAllItems();
    return true;
}

bool PlayerComponent::DropItemToDepositZone() { // this is the bank
	if (inventoryComponent->GetWallet() <= 0) return false;
	inventoryComponent->DepositWalletToQuota();
    return true;
}

bool PlayerComponent::DropItem() {
    if (inventoryComponent->ItemInHand()) {
        if (inDropZone) return DropItemToDropZone();
        else return DropItemToFloor();
    }
    else if (inBank){
        return DropItemToDepositZone();
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

bool PlayerComponent::InteractExit() {
	if (inExit) {
        ExitEvent* e = new ExitEvent();
        EventManager::Call<ExitEvent>(e);
		return true;
	}
	return false;
}

void PlayerComponent::OnItemInteract() {
    if (!inventoryComponent) return;
    if (timeSinceLastPickUp < pickUpDelay) return;
    if (DropItem()) return;
	if (InteractExit()) return;
    TryPickUp();
}

void PlayerComponent::GetWeightModifier() {
    weightModifier = 1.0f;
    float weight = inventoryComponent->GetItemCombinedWeight();
    if (weight == 0) { 
        weightModifier = 1.0f; }
    else {
        weightModifier = 1.0 - (weight / 100);
    }
    return;
}

void PlayerComponent::OnPlayerMove() {
    isMoving = false;
    if (inputComponent->GetNamedAxis("Forward") == 0 && inputComponent->GetNamedAxis("Sidestep") == 0)
        return;
    isMoving = true;
    Vector3 dir;
    Matrix3 yawRotation = inputComponent->GetMouseGameWorldYawMatrix();
    GetWeightModifier();
    dir += yawRotation * Vector3(0, 0, -inputComponent->GetNamedAxis("Forward"));
    dir += yawRotation * Vector3(inputComponent->GetNamedAxis("Sidestep"), 0, 0);
    Matrix3 offsetRotation = Matrix::RotationMatrix3x3(0.0f, Vector3(0, 1, 0));
    dir = offsetRotation * dir;

    physicsObj->AddForce(dir * speed * weightModifier * (isDashing ? dashMultiplier : 1.0f));
    physicsObj->RotateTowardsVelocity();
}

void PlayerComponent::CheckTagStack() {
    while (!collidedTags.empty()) {
        if (collidedTags.top() == Tags::DropZone)
            inDropZone = true;
		else if (collidedTags.top() == Tags::DepositZone)
			inBank = true;
		else if (collidedTags.top() == Tags::Exit)
			inExit = true;
        else if (collidedTags.top() == Tags::Ground)
            isGrounded = true;
        collidedTags.pop();
    }
}

void PlayerComponent::CheckInputStack() {
    while (!inputStack.empty()) {
        if (inputStack.top() == onDashBinding)
            OnDashInput();
        else if (inputStack.top() == onJumpBinding)
            OnJumpInput();
        else if (inputStack.top() == onItemInteractBinding)
            OnItemInteract();
        else if (inputStack.top() == onPauseBinding)
            OnPauseInput();
		else if (inputStack.top() == onDebugBinding)
			OnDebugInput();
        inputStack.pop();
    }
}

void PlayerComponent::UpdateStates(float deltaTime) {
    isDashing = false;
    isGrounded = !isJumping ? true : false;
    inDropZone = false;
	inBank = false;
	inExit = false;
    timeSinceLastPickUp += deltaTime;
}

void PlayerComponent::AddDownWardsVelocity() {
    if (isGrounded) return;
    if (physicsObj->GetLinearVelocity().y <= -0.2f)
        physicsObj->AddForce(Vector3(0, -1, 0) * downwardsVelocityMod);

    Vector3 force = physicsObj->GetForce();
    Vector3 velocity = physicsObj->GetLinearVelocity();

    if (Vector::Length(velocity) > Vector::Length(maxVelocity)) {
        physicsObj->ClearForces();
        physicsObj->SetLinearVelocity(Vector3(
            std::min(maxVelocity.x, velocity.x),
            std::min(maxVelocity.y, velocity.y),
            std::min(maxVelocity.z, velocity.z)));
    }
}

void PlayerComponent::Update(float deltaTime)
{
    if (physicsObj == nullptr || physicsComponent == nullptr || inputComponent == nullptr || staminaComponent == nullptr)
        return;
    CheckTagStack();
    CheckInputStack();

    OnPlayerMove();
    OnJump(deltaTime);
    AddDownWardsVelocity();
    UpdateStates(deltaTime);
}