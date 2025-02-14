#pragma once
#include "PhysicsObject.h"
#include "InputComponent.h"

#include "Ray.h"
#include "Kitten.h"
#include "CollectMe.h"
#include "EventListener.h"

#include "Window.h"
#include "CollisionDetection.h"

namespace NCL {
    namespace CSC8508 {

        class OnJumpEvent : public Event {};
        class PlayerGameObject : public GameObject, public EventListener<OnJumpEvent> {
        public:

            PlayerGameObject();
            ~PlayerGameObject();

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
                physicsComponent = this->TryGetComponent<PhysicsComponent>();
                inputComponent = this->TryGetComponent<InputComponent>();

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

                Vector3 dir;
                yaw -= inputComponent->GetNamedAxis("XLook");

                if (yaw < 0)
                    yaw += 360.0f;
                if (yaw > 360.0f)
                    yaw -= 360.0f;

                Matrix3 yawRotation = Matrix::RotationMatrix3x3(yaw, Vector3(0, 1, 0));

                dir += yawRotation * Vector3(0, 0, -inputComponent->GetNamedAxis("Forward"));
                dir += yawRotation * Vector3(inputComponent->GetNamedAxis("Sidestep"), 0, 0);

                Matrix3 offsetRotation = Matrix::RotationMatrix3x3(-55.0f, Vector3(0, 1, 0));
                dir = offsetRotation * dir;

                physicsObj->AddForce(dir * speed);
                physicsObj->RotateTowardsVelocity();
            }

            void OnCollisionBegin(BoundsComponent* otherBounds) override {
                GameObject& otherObject = otherBounds->GetGameObject();

                if (otherObject.GetTag() == Tags::Enemy)
                    endGame(false);
                else if (otherObject.GetTag() == Tags::Collect)
                {
                    CollectMe& collect = static_cast<CollectMe&>(otherObject);
                    if (!collect.IsCollected())
                    {
                        increaseScore(collect.GetPoints());
                        collect.SetCollected(true);
                        collect.GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));

                    }
                }
            }
 
        protected:
            float speed = 10.0f;
            float	yaw = 0;
            EndGame endGame;
            IncreaseScore increaseScore;

            InputComponent* inputComponent = nullptr;
            PhysicsComponent* physicsComponent = nullptr;
            PhysicsObject* physicsObj = nullptr;
        };
    }
}