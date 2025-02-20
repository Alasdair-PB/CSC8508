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
#include "PhysicsComponent.h"

#include "CollisionDetection.h"

#include "GameWorld.h"
#include "Kitten.h"
#include "UpdateObject.h"


namespace NCL {
    namespace CSC8508 {
        class Swarm : public GameObject {
        public:

            typedef std::function<Vector3()> GetPlayerPos;

            Swarm();
            ~Swarm();
            void SetGetPlayer(GetPlayerPos getPlayerPos) { this->getPlayerPos = getPlayerPos; }

            void AddObjectToSwarm(Kitten* object) { objects.push_back(object); }

            void RemoveObjectFromSwarm(Kitten* object) {
                objects.erase(std::remove(objects.begin(), objects.end(), object), objects.end());
            }

            void OnAwake() override
            {
                physicsComponent = this->TryGetComponent<PhysicsComponent>();
                navMeshComponent = this->TryGetComponent<NavMeshComponent>();
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
            NavMeshComponent* navMeshComponent;
            PhysicsComponent* physicsComponent;

            BehaviourSequence* sequence = nullptr;
            BehaviourState state;

            void MoveObjectsAlongSwarm();
            void  ReduceVelocityOnStop(float roundingPrecision, Vector3 currentPos, vector<Kitten*> objects);


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
                        navMeshComponent->SetPath(playerPos);
                        state = Ongoing;
                    }
                    else if (state == Ongoing)
                    {
                        if (navMeshComponent->AtDestination())
                            return state;
                    }
                    return state;
                }
            );
        };
    }
}
