#pragma once
#include "PhysicsObject.h"
#include "Ray.h"
#include "Kitten.h"
#include "CollectMe.h"

#include "Window.h"

#include "UpdateObject.h"
//#include "Controller.h"
#include "CollisionDetection.h"

namespace NCL {
    namespace CSC8508 {
        class PlayerGameObject : public UpdateObject, public GameObject {
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

            void Update(float dt) override
            {
                if (activeController == nullptr)
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

                this->GetPhysicsObject()->AddForce(dir * speed);
                this->GetPhysicsObject()->RotateTowardsVelocity();
            }

            virtual void OnCollisionBegin(GameObject* otherObject) override {
                if (otherObject->GetTag() == Tags::Enemy) 
                    endGame(false);
                else if (otherObject->GetTag() == Tags::Kitten)
                {
                    if (Window::GetMouse()->ButtonDown(NCL::MouseButtons::Right))
                    {
                        Kitten* kitten = static_cast<Kitten*>(otherObject);
                        kitten->ThrowSelf(Vector::Normalise(this->GetPhysicsObject()->GetLinearVelocity()));
                    }
                }
                else if (otherObject->GetTag() == Tags::Collect)
                {
                    CollectMe* collect = static_cast<CollectMe*>(otherObject); 
                    if (!collect->IsCollected()) 
                    {
                        increaseScore(collect->GetPoints());
                        collect->SetCollected(true);
                        collect->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));

                    }
                }
            }


        protected:
            const Controller* activeController = nullptr;
            float speed = 5.0f;
            float	yaw;
            EndGame endGame;
            IncreaseScore increaseScore;
        };
    }
}