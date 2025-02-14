#pragma once
#include "GameObject.h"

#include "BehaviourNode.h"
#include "BehaviourSelector.h"
#include "BehaviourSequence.h"
#include "BehaviourAction.h"
#include "NavigationMesh.h"

#include "PhysicsObject.h"
#include "Ray.h"
#include "CollisionDetection.h"

#include "GameWorld.h"
#include "UpdateObject.h"

namespace NCL {
    namespace CSC8508 {
        class NavMeshAgent : public GameObject {
        public:
            NavMeshAgent(NavigationMesh* navMesh)
            {
                this->navMesh = navMesh;
            }

            ~NavMeshAgent() {}

            void MoveAlongPath()
            {
                Vector3 pos = this->transform.GetPosition();

                if (outPathIndex < 0 || testNodes.size() < outPathIndex + 1)
                    return;

                if (Vector::Length(pos - testNodes[outPathIndex]) < minWayPointDistanceOffset) {
                    outPathIndex--;

                    if (outPathIndex < 0)
                        return;
                }

                std::cout << outPathIndex << std::endl;
                Vector3 target = testNodes[outPathIndex];
                Vector3 dir = target - pos;

                dir = Vector::Normalise(dir);
                

                auto lastVelocity = this->GetPhysicsObject()->GetLinearVelocity();
                dir *= speed;
                dir.y += 0.1f;
                dir.y = dir.y + lastVelocity.y;
                this->GetPhysicsObject()->SetLinearVelocity(dir);
            }


        protected:
            NavigationMesh* navMesh = nullptr;
            vector<Vector3> testNodes;
            NavigationPath outPath;

            int outPathIndex = 0;
            float speed = 5.0f;
            float minWayPointDistanceOffset = 2;

            Vector3 targetPos = Vector3();

            void DisplayPathfinding(Vector4 colour)
            {
                for (int i = 1; i < testNodes.size(); ++i) {
                    Vector3 a = testNodes[i - 1];
                    Vector3 b = testNodes[i];

                    Debug::DrawLine(a, b, colour);
                }
            }

            bool ExistingPath(NavigationPath lastOutPath, NavigationPath outPath) {
                NavigationPath tempOutPath = outPath;
                bool pathsEqual = true;

                Vector3 lastPos, outPos;

                while (lastOutPath.PopWaypoint(lastPos) && tempOutPath.PopWaypoint(outPos)) {
                    if (lastPos.x != outPos.x || lastPos.y != outPos.y || lastPos.z != outPos.z) {
                        pathsEqual = false;
                        break;
                    }
                }
                if (pathsEqual && (lastOutPath.PopWaypoint(lastPos) || tempOutPath.PopWaypoint(outPos)))
                    pathsEqual = false;

                if (pathsEqual)
                    return true;
                return false;
            }

            const bool smoothPath = false;
            Vector3 lastStart = Vector3(1,1,1);
            Vector3 lastEnd = Vector3(1,1,1);
            const float offset = 5.0f;

            void SetPath(Vector3 startPos, Vector3 endPos)
            {
                if (testNodes.size() != 0 && Vector::Length(lastEnd - endPos) < offset &&
                    Vector::Length(lastStart - startPos) < offset)
                    return;

                outPath.clear();
                bool found = navMesh->FindPath(startPos, endPos, outPath);

               if (!found)
                    return;

                if (smoothPath)
                    navMesh->SmoothPath(outPath);
 
                Vector3 pos;
                testNodes.clear();
           
                if (smoothPath) {
                    while (outPath.PopWaypoint(pos)) {
                        testNodes.push_back(pos);
                    }
                }
                else {     
                    std::stack<Vector3> tempStack;
                    while (outPath.PopWaypoint(pos)) {
                        tempStack.push(pos);
                    }
                    while (!tempStack.empty()) {
                        testNodes.push_back(tempStack.top());
                        tempStack.pop();
                    }
                }

                if (testNodes.size() >= 2 &&
                    (Vector::Length(this->GetTransform().GetPosition() - testNodes[testNodes.size() - 2])
                        < Vector::Length(testNodes[testNodes.size() - 2] - testNodes[testNodes.size() - 1])))
                    outPathIndex = testNodes.size() - 2;
                else
                    outPathIndex = testNodes.size() - 1;

                targetPos = endPos;
                lastEnd = endPos;
                lastStart = startPos;
            }
        };
    }
}
