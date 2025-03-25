#pragma once
#include "IComponent.h"
#include "Ray.h"

//#include "GameWorld.h"
//#include "ComponentManager.h"

namespace NCL {
    namespace CSC8508 {

        class SightComponent : public IComponent {
        public:
            SightComponent(GameObject& g) :/*worldInstance(GameWorld::Instance()),*/ IComponent(g) {}
            ~SightComponent();

           /* template<typename T>requires
                std::is_base_of_v<IComponent, T> 
            GameObject* GetClosestVisibleTarget(float maxRange, Vector3 rayOffset) {
                Ray ray;
                RayCollision* closestCollision;
                GameObject* closetsGamObj = nullptr;
                float closestDist = -1.0f;
                Vector3 position = GetGameObject().GetTransform().GetPosition() + rayOffset;

                ComponentManager::OperateOnBufferContents<T>(
                    [&position, &closestDist, &closetsGamObj, &ray, &closestCollision](T* o) {
                        if (!(o->IsEnabled()))
                            return;

                        Vector3 oPositon = o->GetGameObject().GetTransform().GetPosition();
                        Vector3 objToTarget = (oPositon - position);

                        auto len = Vector::Length(objToTarget);
                        if (len > maxRange && (len < closestDist || closestDist == -1.0f)) { 
                            ray = Ray(position, objToTarget);
                            BoundsComponent* bounds = o->GetGameObject().TryGetComponent<BoundsComponent>();
                            if (worldInstance.Raycast(ray, closestCollision, true, bounds)) {
                                closestDist = len;
                                closetsGamObj = &o->GetGameObject();
                            }
                        }
                    }
                );
                return closetsGamObj;
            }*/

            template<typename T>requires
                std::is_base_of_v<IComponent, T>
            T* CanSeeComponent(float range, Vector3 rayOffset, Vector3 direction) {

                Vector3 position = GetGameObject().GetTransform().GetPosition() + rayOffset;
                Ray ray = Ray(position, Vector::Normalise(direction) * range);
                RayCollision closestCollision;
                T* visibleComponent = nullptr;
                BoundsComponent* bounds = GetGameObject().TryGetComponent<BoundsComponent>();
                /*if (worldInstance.Raycast(ray, closestCollision, true, bounds)) {

                    if ((BoundsComponent*)closestCollision.node) {
                        T* component = ((BoundsComponent*)closestCollision.node)->GetGameObject().TryGetComponent<T>();
                        if (component)
                            return component;
                    }
                }*/
                return visibleComponent;
            }

        protected:
            //GameWorld& worldInstance;
        };
    }
}