#pragma once
#include "PhysicsObject.h"
#include "Ray.h"
#include "Kitten.h"
#include "CollectMe.h"
#include "Window.h"
#include "CollisionDetection.h"

namespace NCL {
    namespace CSC8508 {
        class PlayerGameObject : public GameObject {
        public:

            PlayerGameObject();
            ~PlayerGameObject();

            typedef std::function<void(bool hasWon)> EndGame;
            typedef std::function<void(float points)> IncreaseScore;


            void SetEndGame(EndGame endGame) {
                this->endGame = endGame;
            }

            void SetIncreaseScore(IncreaseScore increaseScore) {
                this->increaseScore = increaseScore;
            }

            void SetController(const Controller& c) {
                activeController = &c;
            }

           /**
            * Function invoked each frame after Update.
            * @param deltaTime Time since last frame
            */
            void OnAwake() override
            {
                physicsComponent = this->TryGetComponent<PhysicsComponent>();

                if (physicsComponent)
                    physicsObj = physicsComponent->GetPhysicsObject();
            }

            /**
             * Function invoked each frame.
             * @param deltaTime Time since last frame
             */
            void Update(float deltaTime) override
            {
                if (activeController == nullptr || physicsObj == nullptr)
                    return;

                Vector3 dir;
                yaw -= activeController->GetNamedAxis("XLook");

                if (yaw < 0)
                    yaw += 360.0f;
                if (yaw > 360.0f)
                    yaw -= 360.0f;

                Matrix3 yawRotation = Matrix::RotationMatrix3x3(yaw, Vector3(0, 1, 0));

                dir += yawRotation * Vector3(0, 0, -activeController->GetNamedAxis("Forward"));
                dir += yawRotation * Vector3(activeController->GetNamedAxis("Sidestep"), 0, 0);

                Matrix3 offsetRotation = Matrix::RotationMatrix3x3(-45.0f, Vector3(0, 1, 0));
                dir = offsetRotation * dir;

                physicsObj->AddForce(dir * speed);
                physicsObj->RotateTowardsVelocity();
            }

            void OnCollisionBegin(BoundsComponent* otherBounds) override {
                GameObject& otherObject = otherBounds->GetGameObject();

                if (otherObject.GetTag() == Tags::Enemy)
                    endGame(false);
                else if (otherObject.GetTag() == Tags::Kitten)
                {
                    if (Window::GetMouse()->ButtonDown(NCL::MouseButtons::Right))
                    {
                        Kitten& kitten = static_cast<Kitten&>(otherObject);
                        kitten.ThrowSelf(Vector::Normalise(physicsObj->GetLinearVelocity()));
                    }
                }
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
            const Controller* activeController = nullptr;
            float speed = 5.0f;
            float	yaw;
            EndGame endGame;
            IncreaseScore increaseScore;

            PhysicsComponent* physicsComponent;
            PhysicsObject* physicsObj;
        };
    }
}