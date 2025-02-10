#pragma once

#include "GameObject.h"

#include "BehaviourNode.h"
#include "BehaviourSelector.h"
#include "BehaviourSequence.h"
#include "BehaviourAction.h"
#include "NavigationMesh.h"

#include "PhysicsObject.h"
#include "Ray.h"

#include "NavMeshComponent.h"
#include "CollisionDetection.h"
#include "GameWorld.h"

namespace NCL {
    namespace CSC8508 {
        class Kitten : public GameObject {
        public:
            Kitten(GameObject* swarm);

            ~Kitten();

            /**
            * Function invoked each frame after Update.
            * @param deltaTime Time since last frame
            */
            void OnAwake() override 
            {
                physicsComponent = this->TryGetComponent<PhysicsComponent>();
                navMeshComponent = this->TryGetComponent<NavMeshComponent>();
            }

            /**
             * Function invoked each frame.
             * @param deltaTime Time since last frame
             */
            void Update(float deltaTime) override
            {
                if (!alive) {
                    selected = false;
                    yearnsForTheSwarm = false;
                    return;
                }

                if (state != Ongoing) {
                    sequence->Reset();
                    state = Ongoing;
                }

                if (selected) {
                    state = sequence->Execute(deltaTime);
                    navMeshComponent->MoveAlongPath();
                }
            }


            void OnCollisionBegin(BoundsComponent* otherBounds) override {
                if (!otherBounds)
                    return;
                GameObject& otherObject = otherBounds->GetGameObject();
                if (otherObject.GetTag() == Tags::CursorCast) {
                    selected = true;
                }
                if (otherObject.GetTag() == Tags::Enemy) {
                    alive = false;
                    Quaternion rot = Quaternion::AxisAngleToQuaterion(Vector3(1, 0, 0), 90);
                    this->GetTransform().SetOrientation(rot);
                    this->GetRenderObject()->SetColour(Vector4(1.0f, 0, 0, 1));
                }
            }

            void ThrowSelf(Vector3 dir);
            void SetSelected(bool state) { selected = state; }
            bool GetSelected() { return selected; }
            bool GetYearnsForSwarm() { return yearnsForTheSwarm; }

        protected:

            BehaviourSequence* sequence = nullptr;
            BehaviourState state;
            GameObject* swarmCenter = nullptr;
            PhysicsComponent* physicsComponent = nullptr;
            NavMeshComponent* navMeshComponent = nullptr;

            bool alive = true;
            bool selected;
            bool yearnsForTheSwarm = false;


            BehaviourAction* idle = new BehaviourAction("Idle",
                [&](float dt, BehaviourState state) -> BehaviourState
                {
                    if (state == Initialise) {
                        state = Ongoing;
                        yearnsForTheSwarm = false;

                    }
                    else if (state == Ongoing && selected)
                        return Success;
                    return state;
                }
            );

            BehaviourAction* goToSwarm = new BehaviourAction("GoToSwarm",
                [&](float dt, BehaviourState state) -> BehaviourState
                {
                    Vector3 pos = this->transform.GetPosition();
                    Vector3 swarmPos  = swarmCenter->GetTransform().GetPosition();

                    if (state == Initialise) {
                        navMeshComponent->SetPath(swarmPos);
                        yearnsForTheSwarm = false;
                        state = Ongoing;
                    }
                    else if (state == Ongoing)
                    {
                        if (Vector::Length(pos - swarmPos) < 6.0f)
                        {
                            yearnsForTheSwarm = true;
                            navMeshComponent->ClearPath();
                            return Success;
                        }

                        if (navMeshComponent->AtDestination()) {
                            navMeshComponent->SetPath(swarmPos);
                            return state;
                        }
                    }
                    return state;
                }
            );


            BehaviourAction* followSwarm = new BehaviourAction("FollowSwarm",
                [&](float dt, BehaviourState state) -> BehaviourState
                {
                    Vector3 pos = this->transform.GetPosition();
                    Vector3 swarmPos = swarmCenter->GetTransform().GetPosition();

                    if (state == Initialise) {
                        navMeshComponent->ClearPath();
                        state = Ongoing;
                    }
                    else if (state == Ongoing)
                    {
                        if (Vector::Length(pos - swarmPos) > 10.0f) {
                            navMeshComponent->ClearPath();
                            yearnsForTheSwarm = false;
                            selected = false;
                            return Failure;
                        }
                    }
                    return state;
                }
            );
        };
    }
}