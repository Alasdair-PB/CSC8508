#pragma once

#include "GameObject.h"

#include "BehaviourNode.h"
#include "BehaviourSelector.h"
#include "BehaviourSequence.h"
#include "BehaviourAction.h"
#include "NavigationMesh.h"

#include "PhysicsObject.h"
#include "Ray.h"
#include "UpdateObject.h"

#include "NavMeshAgent.h"
#include "CollisionDetection.h"
#include "GameWorld.h"

namespace NCL {
    namespace CSC8508 {
        class Kitten : public NavMeshAgent, public UpdateObject {
        public:

            Kitten(NavigationMesh* navMesh, GameObject* swarm);
            ~Kitten();
            void SetSelected(bool state) { selected = state; }
            bool GetSelected() { return selected; }
            bool GetYearnsForSwarm() { return yearnsForTheSwarm; }

            void Update(float dt) override
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
                    state = sequence->Execute(dt);
                    //DisplayPathfinding(Vector4(0, 1, 0, 1));
                    MoveAlongPath();
                }
            }

            virtual void OnCollisionBegin(GameObject* otherObject) override {
                if (otherObject->GetTag() == Tags::CursorCast) {
                    selected = true;
                }
                if (otherObject->GetTag() == Tags::Enemy) {
                    alive = false;
                    Quaternion rot = Quaternion::AxisAngleToQuaterion(Vector3(1, 0, 0), 90);
                    this->GetTransform().SetOrientation(rot);
                    this->GetRenderObject()->SetColour(Vector4(1.0f, 0, 0, 1));
                    this->GetPhysicsObject()->SetLinearVelocity(Vector3(0, 0, 0));
                }
            }

            void ThrowSelf(Vector3 dir);


        protected:

            BehaviourSequence* sequence = nullptr;
            BehaviourState state;
            GameObject* swarmCenter = nullptr;

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
                        SetPath(pos, swarmPos);
                        testNodes.insert(testNodes.begin(), swarmPos);
                        yearnsForTheSwarm = false;
                        state = Ongoing;
                    }
                    else if (state == Ongoing)
                    {
                        if (Vector::Length(pos - swarmPos) < 6.0f)
                        {
                            yearnsForTheSwarm = true;
                            testNodes.clear();
                            return Success;
                        }

                        if (Vector::Length(pos - testNodes[0]) < minWayPointDistanceOffset) {
                            SetPath(pos, swarmPos);
                            testNodes.insert(testNodes.begin(), swarmPos);
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
                        testNodes.clear();
                        state = Ongoing;
                    }
                    else if (state == Ongoing)
                    {
                        if (Vector::Length(pos - swarmPos) > 10.0f) {
                            testNodes.clear();
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