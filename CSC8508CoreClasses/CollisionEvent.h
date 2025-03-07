//
// Contributors: Alfie
//

#ifndef COLLISIONEVENT_H
#define COLLISIONEVENT_H
#include "CollisionDetection.h"
#include "Event.h"

using namespace NCL;

struct CollisionEvent : Event {
    GameObject const & object1, & object2;
    CollisionDetection::CollisionInfo const& collisionInfo;

    CollisionEvent(GameObject& object1, GameObject& object2, CollisionDetection::CollisionInfo& collisionInfo)
        : object1(object1), object2(object2), collisionInfo(collisionInfo) { }
};

#endif //COLLISIONEVENT_H
