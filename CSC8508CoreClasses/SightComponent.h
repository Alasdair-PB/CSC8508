#pragma once
#include "IComponent.h"
#include "Ray.h"

#include "GameWorld.h"
//#include "ComponentManager.h"


namespace NCL {
    namespace CSC8508 {

        class SightComponent : public IComponent {
        public:
            SightComponent(GameObject& g) : worldInstance(GameWorld::Instance()), IComponent(g) {}
            ~SightComponent() = default;

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
                T* visibleComponent = nullptr;

                 Vector3 position = GetGameObject().GetTransform().GetPosition() + rayOffset;
                 Vector3 rayVector = Vector::Normalise(direction);
                 Ray ray = Ray(position, rayVector);
                 RayCollision closestCollision;

                 if (worldInstance.Raycast(ray, closestCollision, true)) {
                     if ((BoundsComponent*)closestCollision.node) {
                         GameObject& collidedObject = ((BoundsComponent*)closestCollision.node)->GetGameObject();
                         T* component = collidedObject.TryGetComponent<T>();
                         if (component) {
                             Vector3 componentPosition = collidedObject.GetTransform().GetPosition();
                             if (Vector::Length(position - componentPosition) <= range)
                                return component;
                         }
                     }
                 }
                return visibleComponent;
            }

        protected:
            GameWorld& worldInstance;
        };
    }
}