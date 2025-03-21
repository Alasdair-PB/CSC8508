#pragma once
#include "PhysicsObject.h"
#include "GameObject.h"
#include "InputComponent.h"
#include "StaminaComponent.h"
#include "CollisionEvent.h"

#include "Ray.h"
#include "EventListener.h"
#include "EventManager.h"

#include "Window.h"
#include "CollisionDetection.h"

namespace NCL {
    namespace CSC8508 {
        using namespace NCL::CSC8508::Tags;
        class PlayerComponent : public IComponent, public EventListener<InputButtonEvent>, public EventListener<CollisionEvent> {
        public:

            PlayerComponent(GameObject& gameObject);
            ~PlayerComponent();

            void SetBindingJump(uint32_t j, StaminaComponent* component) {
                onJumpBinding = j;
                if (component) component->SetStaminaAction(j, jumpStamCost);
            }

            void SetBindingDash(uint32_t d, StaminaComponent* component) {
                onDashBinding = d;
                if (component) component->SetStaminaAction(d, dashTickStam);
            }

            void SetBindingPickUp(uint32_t p) {
                onItemPickUpBinding = p;
            }

            void OnEvent(InputButtonEvent* buttonEvent) override {
                if (&buttonEvent->owner != &GetGameObject()) { return; }
                inputStack.push(buttonEvent->buttonId);
            }

            void OnEvent(CollisionEvent* collisionEvent) override
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
            void OnAwake() override
            {
                physicsComponent = GetGameObject().TryGetComponent<PhysicsComponent>();
                inputComponent = GetGameObject().TryGetComponent<InputComponent>();
                staminaComponent = GetGameObject().TryGetComponent<StaminaComponent>();

                EventManager::RegisterListener<InputButtonEvent>(this);
                EventManager::RegisterListener<CollisionEvent>(this);

                if (physicsComponent)
                    physicsObj = physicsComponent->GetPhysicsObject();
            }

            void OnDashInput() {
                if (staminaComponent->PerformActionIfAble(onDashBinding))             
                    isDashing = true;
            }

            void SetLinearVelocity(float jumpDuration) {
                Vector3 velocity = physicsObj->GetLinearVelocity();
                physicsObj->SetLinearVelocity(Vector3(velocity.x, jumpForce * (1 + (0.2f-jumpDuration)), velocity.z));
            }

            void OnJumpInput() {
                if (isJumping || !isGrounded) return;
                if (staminaComponent->PerformActionIfAble(onJumpBinding)) {
                    isJumping = true;
                    jumpDuration = 0;
                    SetLinearVelocity(0);
                }
            }

           void OnJump(float deltaTime) {
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

            void OnItemPickUp() {
                std::cout << "pick up item" << "\n";
            }

            void OnPlayerMove() {
                if (inputComponent->GetNamedAxis("Forward") == 0 && inputComponent->GetNamedAxis("Sidestep") == 0)
                    return;

                Vector3 dir;
                Matrix3 yawRotation = inputComponent->GetMouseGameWorldYawMatrix();

                dir += yawRotation * Vector3(0, 0, -inputComponent->GetNamedAxis("Forward"));
                dir += yawRotation * Vector3(inputComponent->GetNamedAxis("Sidestep"), 0, 0);

                Matrix3 offsetRotation = Matrix::RotationMatrix3x3(0.0f, Vector3(0, 1, 0));
                dir = offsetRotation * dir;


                physicsObj->AddForce(dir * speed * (isDashing ? dashMultiplier : 1.0f));
                physicsObj->RotateTowardsVelocity();
            }

            void CheckInputStack() {
                while (!inputStack.empty()) {
                    if (inputStack.top() == onDashBinding)
                        OnDashInput();
                    else if (inputStack.top() == onJumpBinding)
                        OnJumpInput();
                    else if (inputStack.top() == onItemPickUpBinding)
                        OnItemPickUp();
                    inputStack.pop();
                }
            }

            /**
             * Function invoked each frame.
             * @param deltaTime Time since last frame
             */
            void Update(float deltaTime) override
            {
                if (physicsObj == nullptr || physicsComponent == nullptr || inputComponent == nullptr || staminaComponent == nullptr)
                    return;
                CheckInputStack();
                OnJump(deltaTime);
                OnPlayerMove();
                isDashing = false;
                isGrounded = false;
            }
 
        protected:
            float speed = 15.0f;
            float dashMultiplier = 1.5f;
            float jumpForce = 10.0f;
            float jumpDuration = 0;
            Transform& transform; 
  
            InputComponent* inputComponent = nullptr;
            StaminaComponent* staminaComponent = nullptr;
            PhysicsComponent* physicsComponent = nullptr;
            PhysicsObject* physicsObj = nullptr;

            uint32_t onJumpBinding;
            uint32_t onDashBinding;
            uint32_t onItemPickUpBinding;
            std::stack<uint32_t> inputStack; 

            float downwardsVelocityMod = 50.0f;
            float dashTickStam = 2.0f;
            float jumpStamCost = 10.0f;

            bool isGrounded;
            bool isJumping;
            bool isDashing;
        };
    }
}