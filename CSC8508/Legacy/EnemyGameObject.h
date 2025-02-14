#pragma once
#include "GameObject.h"

#include "BehaviourNode.h"
#include "BehaviourSelector.h"
#include "BehaviourSequence.h"
#include "BehaviourAction.h"
#include "NavigationMesh.h"

#include "PhysicsObject.h"
#include "Ray.h"
#include "NavMeshAgent.h"

#include "CollisionDetection.h"

#include "GameWorld.h"
#include "UpdateObject.h"


namespace NCL {
    namespace CSC8508 {
        class EnemyGameObject : public NavMeshAgent, public UpdateObject {
        public:    

            typedef std::function<bool(Ray& r, float rayDistance)> RaycastToWorld; 
            typedef std::function<Vector3()> GetPlayerPos;

            EnemyGameObject(NavigationMesh* navMesh);
            ~EnemyGameObject();

            void SetRay(RaycastToWorld rayHit){ this->rayHit = rayHit; }
            void SetGetPlayer(GetPlayerPos getPlayerPos) { this->getPlayerPos = getPlayerPos; }

            void Update(float dt) override 
            {
                if (state != Ongoing) {
                    sequence->Reset();
                    state = Ongoing;
                }

                state = sequence->Execute(dt);
                DisplayPathfinding(Vector4(0, 0, 1, 1));
                MoveAlongPath();
                this->GetPhysicsObject()->RotateTowardsVelocity(-90);
            }

        protected:
 
            bool CanSeePlayer();

            RaycastToWorld rayHit;
            GetPlayerPos getPlayerPos;

            BehaviourSequence* sequence = nullptr;
            BehaviourState state;

            const int wayPointsLength = 4;
   
            Vector3 wayPoints[4] = { 
                Vector3(25, 100, 30),
                Vector3(50, 22, -50),
                Vector3(-50, 22, 0),
                Vector3(25, 22, 50)
            };

            float playerDis = 0.0f;
            const float yOffSet = 0.1f; 
            bool playerVisible;

            int wayPointIndex = 0;
            float timer = 0;


            BehaviourAction* patrol = new BehaviourAction("Patrol",
                [&](float dt, BehaviourState state) -> BehaviourState
                {
                    Vector3 pos = this->transform.GetPosition();
                    Vector3 playerPos = getPlayerPos();

                    if (state == Initialise)
                    {
                        SetPath(pos, wayPoints[wayPointIndex]);
                        state = Ongoing;
                        timer = 0;
                    }
                    else if (state == Ongoing)
                    {   
                        timer += dt;

                        if (timer > 1.0f) {
                            if (CanSeePlayer())
                            {
                                wayPointIndex = 0;
                                return Success;
                            }
                            timer = 0;
                        }

                        if (testNodes.size() <= 0) {
                            SetPath(pos, wayPoints[wayPointIndex]);
                            return state;
                        }

                        if (Vector::Length(pos - testNodes[0]) < minWayPointDistanceOffset) {
                            wayPointIndex >= wayPointsLength ? wayPointIndex = 0 : wayPointIndex++;
                            SetPath(pos, wayPoints[wayPointIndex]);
                        }

                    }
                    return state;
                }
            );

            BehaviourAction* chase = new BehaviourAction("Chase",
                [&](float dt, BehaviourState state) -> BehaviourState
                {
                    Vector3 pos = this->transform.GetPosition();
                    Vector3 playerPos = getPlayerPos();

                    if (state == Initialise) {
                        SetPath(pos, playerPos);
                        testNodes.insert(testNodes.begin(), playerPos);
                        outPathIndex++;
                        state = Ongoing;
                    }
                    else if (state == Ongoing)
                    {  
                        if (CanSeePlayer()) {
                            SetPath(pos, playerPos);
                            testNodes.insert(testNodes.begin(), playerPos);
                            outPathIndex++;

                            return state;
                        }
                        else if (Vector::Length(pos - testNodes[0]) < minWayPointDistanceOffset) 
                            return  Failure;
                    }
                    return state;
                }
            );
        };
    }
}
