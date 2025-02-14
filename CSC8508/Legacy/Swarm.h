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
#include "Kitten.h"
#include "UpdateObject.h"


namespace NCL {
    namespace CSC8508 {
        class Swarm : public NavMeshAgent, public UpdateObject {
        public:

            typedef std::function<Vector3()> GetPlayerPos;

            Swarm(NavigationMesh* navMesh);
            ~Swarm();
            void SetGetPlayer(GetPlayerPos getPlayerPos) { this->getPlayerPos = getPlayerPos; }

            void AddObjectToSwarm(Kitten* object) { objects.push_back(object); }

            void RemoveObjectFromSwarm(Kitten* object) {
                objects.erase(std::remove(objects.begin(), objects.end(), object), objects.end());
            }

            void Update(float dt) override {
                this->GetTransform().SetPosition(getPlayerPos());
                MoveObjectsAlongSwarm();
            }

        protected:

            struct BoidRules {
                float minDistanceRule2 = 2.0f;
                float minDistanceRule3 = 0.5f;
                float forceMultiplier = 1.0f;
                float maxDistanceToCenter = 9.0f;

                float rule1Weight = 1.0f;
                float rule2Weight = 1.0f;
                float rule3Weight = 0.0f;
                int roundingPrecision = 1;
            };



            BoidRules ruleConfig;
            GetPlayerPos getPlayerPos;

            BehaviourSequence* sequence = nullptr;
            BehaviourState state;

            void MoveObjectsAlongSwarm();

            Vector3 rule1(Kitten*& b, std::vector<Kitten*>& boids);
            Vector3 rule2(Kitten*& b, std::vector<Kitten*>& boids);
            Vector3 rule3(Kitten*& b, std::vector<Kitten*>& boids);

            vector<Kitten*> objects;

            BehaviourAction* chase = new BehaviourAction("FollowPlayer",
                [&](float dt, BehaviourState state) -> BehaviourState
                {
                    Vector3 pos = this->transform.GetPosition();
                    Vector3 playerPos = getPlayerPos();

                    if (state == Initialise)
                    {
                        SetPath(pos, playerPos);
                        state = Ongoing;
                    }
                    else if (state == Ongoing)
                    {
                        if (testNodes.size() == 0)
                            return state;

                        if (!(Vector::Length(pos - testNodes[0]) < minWayPointDistanceOffset)) 
                            SetPath(pos, playerPos);
                    }
                    return state;
                }
            );
        };
    }
}
