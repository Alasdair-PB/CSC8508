#pragma once
#include "IComponent.h"
#include "GameWorld.h"

#include "ComponentManager.h"

namespace NCL {
    namespace CSC8508 {

        class SightComponent : public IComponent {
        public:
            SightComponent(GameObject& g) :worldInstance(GameWorld::Instance()), IComponent(g) {
                playerVisible = false;
            }
            ~SightComponent();

            template<typename T>requires std::is_base_of_v<IComponent, T> GameObject* GetClosestVisibleTarget(float maxRange, Vector3 rayOffset) {

                Ray ray;
                RayCollision* closestCollision;
                GameObject* closetsGamObj = nullptr;
                float closestDist = -1.0f;
                Vector3 position = GetGameObject().GetTransform().GetPosition();


                ComponentManager::OperateOnBufferContents<T>(
                    [](T* o) {
                        if (!(o->IsEnabled()))
                            return;
                        Vector3 oPositon = o->GetGameObject().GetTransform().GetPosition();
                        Vector3 objToTarget = (oPositon - position);
                        auto len = Vector::Length(objToTarget);
                        if (len > maxRange && (len < closestDist || closestDist == -1.0f)) {

                            BoundsComponent* bounds = GetGameObject().TryGetComponent<BoundsComponent>();
                            if (worldInstance.Raycast(ray,closestCollision, true, bounds)) {
                                closestDist = len;
                                closetsGamObj = o->GetGameObject();// Might need to be & or *
                            }
                        }

                    }
                );
                return closetsGamObj;
            }

            void OnAwake() override {

            }

        protected:
            GameWorld& worldInstance;
            
            
            bool playerVisible;
            float playerDis = 0.0f;

        };
    }
}