#pragma once
#include "PhysicsObject.h"
#include "GameObject.h"
#include "InputComponent.h"
#include "../Attributes/StaminaComponent.h"
#include "CollisionEvent.h"
#include "SightComponent.h"
#include "../Attributes/DamageableComponent.h"
#include "EventListener.h"
#include "EventManager.h"
#include "CollisionDetection.h"
#include "../Inventory/InventoryNetworkManagerComponent.h"
#include "UISystem.h"

namespace NCL {
    namespace CSC8508 {
        using namespace Tags;

        class ExitEvent : public Event {
        public:
            ExitEvent() {}
        };

		class PauseEvent : public Event {
		public:
			PauseEvent() {}
		};

		class DebugEvent : public Event {
		public:
			DebugEvent() {}
		};


        class PlayerComponent : public IComponent, public EventListener<InputButtonEvent>, public EventListener<CollisionEvent>, public EventListener<DeathEvent> {
        public:

            PlayerComponent(GameObject& gameObject);
            ~PlayerComponent();

            void SetBindingJump(uint32_t j, StaminaComponent* component);
            void SetBindingDash(uint32_t d, StaminaComponent* component);
            void SetBindingInteract(uint32_t p);
            void SetBindingScrollInventory(uint32_t p);
            void SetBindingPause(uint32_t p);
			void SetBindingDebug(uint32_t d);

            void OnEvent(InputButtonEvent* buttonEvent) override;
            void OnEvent(CollisionEvent* collisionEvent) override;
            void OnEvent(DeathEvent* deathEvent) override;

            void OnAwake() override;
            void Update(float deltaTime) override;

			bool IsGrounded() { return isGrounded; }
            bool IsJumping() { return isJumping; }
            bool IsMoving() { return isMoving; }

        protected:
            void OnDashInput();
            void OnJumpInput();
            void OnItemInteract();
            void OnPlayerMove();
            void OnPauseInput();
			void OnDebugInput();

            void SetLinearVelocity(float jumpDuration);
            void OnJump(float deltaTime);
            void AddDownWardsVelocity();
            void TryPickUp();
            bool DropItem();

            bool DropItemToFloor();
            bool DropItemToDropZone();
			bool DropItemToDepositZone();
            bool InteractExit();
            void PickUpItem(ItemComponent* item);
            void CheckTagStack();
            void CheckInputStack();
            void UpdateStates(float deltaTime);
            bool CheckTag(Tag tag, CollisionEvent* collisionEvent);
            Vector3 GetForwardsDirection();
            void GetWeightModifier();

            float speed = 3.75f;
            float dashMultiplier = 1.75f;
            float jumpForce = 10.0f;
            float jumpDuration = 0;
            float downwardsVelocityMod = 50.0f;
            float dashTickStam = 0.3f;
            float jumpStamCost = 10.0f;
            float visibleRange = 5.0f;
            float weightModifier = 1.0f;

            float timeSinceLastPickUp;
            float pickUpDelay;

            bool isGrounded;
            bool isJumping;
            bool isDashing;
            bool isMoving;
            bool inDropZone;
            bool inBank;
            bool inExit;


            Vector3 visionHeight = Vector3(0, 0.3f, 0);
            Vector3 maxVelocity = Vector3(15.0f, 15.0f, 15.0f);
            float maxForce = 15.0f;
            std::stack<uint32_t> inputStack; 
            std::stack<Tag> collidedTags;

            Transform& transform; 
            SightComponent* sightComponent = nullptr;
            InputComponent* inputComponent = nullptr;
            StaminaComponent* staminaComponent = nullptr;
            PhysicsComponent* physicsComponent = nullptr;
            PhysicsObject* physicsObj = nullptr;
            InventoryManagerComponent* inventoryComponent = nullptr;

            uint32_t onJumpBinding;
            uint32_t onDashBinding;
            uint32_t onItemInteractBinding;
            uint32_t onInvScrollBinding;
			uint32_t onPauseBinding;
			uint32_t onDebugBinding;
        };
    }
}