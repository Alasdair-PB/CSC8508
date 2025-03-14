#pragma once
#include "PhysicsObject.h"
#include "GameObject.h"
#include "InputComponent.h"

#include "Ray.h"
#include "EventListener.h"

#include "Window.h"
#include "CollisionDetection.h"

namespace NCL {
    namespace CSC8508 {

        class OnJumpEvent : public Event {};
        class PlayerComponent : public IComponent, public EventListener<OnJumpEvent> {
        public:

            PlayerComponent(GameObject& gameObject);
            ~PlayerComponent();

            typedef std::function<void(bool hasWon)> EndGame;
            typedef std::function<void(float points)> IncreaseScore;

            void OnEvent(OnJumpEvent* e) override
            {
                // do math stuff
            }

            void SetEndGame(EndGame endGame) {
                this->endGame = endGame;
            }

           /**
            * Function invoked each frame after Update.
            * @param deltaTime Time since last frame
            */
            void OnAwake() override
            {
                physicsComponent = GetGameObject().TryGetComponent<PhysicsComponent>();
                inputComponent = GetGameObject().TryGetComponent<InputComponent>();

                if (physicsComponent)
                    physicsObj = physicsComponent->GetPhysicsObject();
            }

            /**
             * Function invoked each frame.
             * @param deltaTime Time since last frame
             */
            void Update(float deltaTime) override
            {
                if (physicsObj == nullptr || physicsComponent == nullptr || inputComponent == nullptr)
                    return;

                if (inputComponent->GetNamedAxis("Forward") == 0 && inputComponent->GetNamedAxis("Sidestep") == 0)
                    return;

                Vector3 dir;
                Matrix3 yawRotation = inputComponent->GetMouseGameWorldYawMatrix();

                dir += yawRotation * Vector3(0, 0, -inputComponent->GetNamedAxis("Forward"));
                dir += yawRotation * Vector3(inputComponent->GetNamedAxis("Sidestep"), 0, 0);

                Matrix3 offsetRotation = Matrix::RotationMatrix3x3(0.0f, Vector3(0, 1, 0));
                dir = offsetRotation * dir;

                physicsObj->AddForce(dir * speed);
                physicsObj->RotateTowardsVelocity();
            }
 
        protected:
            float speed = 10.0f;
            EndGame endGame;
            IncreaseScore increaseScore;

            InputComponent* inputComponent = nullptr;
            PhysicsComponent* physicsComponent = nullptr;
            PhysicsObject* physicsObj = nullptr;
        };
    }
}