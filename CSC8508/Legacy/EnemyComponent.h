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
        class EnemyComponent: public IComponent {
        public:    

            typedef std::function<bool(Ray& r, float rayDistance)> RaycastToWorld; 
            typedef std::function<Vector3()> GetPlayerPos;

            EnemyComponent(GameObject& gameObject, NavigationMesh* navMesh);
            ~EnemyComponent();

            void SetRay(RaycastToWorld rayHit){ this->rayHit = rayHit; }
            void SetGetPlayer(GetPlayerPos getPlayerPos) { this->getPlayerPos = getPlayerPos; }

            /**
          * Function invoked each frame after Update.
          * @param deltaTime Time since last frame
          */
            void OnAwake() override
            {
                navMeshComponent = GetGameObject().TryGetComponent<NavMeshComponent>();
                physicsComponent = GetGameObject().TryGetComponent<PhysicsComponent>();

            }

            void Update(float dt) override 
            {
                if (state != Ongoing) {
                    sequence->Reset();
                    state = Ongoing;
                }

                state = sequence->Execute(dt);
                navMeshComponent->DisplayPathfinding(Vector4(0, 0, 1, 1));
                navMeshComponent->MoveAlongPath();
                physicsComponent->GetPhysicsObject()->RotateTowardsVelocity(-90);
            }

        protected:
 
            bool CanSeePlayer();

            RaycastToWorld rayHit;
            GetPlayerPos getPlayerPos;
            NavMeshComponent* navMeshComponent = nullptr;
            PhysicsComponent* physicsComponent = nullptr;

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
                    Vector3 pos = GetGameObject().GetTransform().GetPosition();
                    Vector3 playerPos = getPlayerPos();

                    if (state == Initialise)
                    {
                        navMeshComponent->SetPath(wayPoints[wayPointIndex]);
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

                        if (navMeshComponent->AtDestination()) {
                            wayPointIndex >= wayPointsLength ? wayPointIndex = 0 : wayPointIndex++;
                            navMeshComponent->SetPath(wayPoints[wayPointIndex]);
                        }

                    }
                    return state;
                }
            );

            BehaviourAction* chase = new BehaviourAction("Chase",
                [&](float dt, BehaviourState state) -> BehaviourState
                {
                    Vector3 pos = GetGameObject().GetTransform().GetPosition();
                    Vector3 playerPos = getPlayerPos();

                    if (state == Initialise) {
                        navMeshComponent->SetPath(playerPos);
                        state = Ongoing;
                    }
                    else if (state == Ongoing)
                    {  
                        if (CanSeePlayer()) {
                            navMeshComponent->SetPath(playerPos);
                            return state;
                        }
                        else if (navMeshComponent->AtDestination())
                            return  Failure;
                    }
                    return state;
                }
            );
        };
    }
}
