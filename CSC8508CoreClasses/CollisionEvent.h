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

    CollisionEvent(GameObject const& object1, GameObject const& object2, CollisionDetection::CollisionInfo const& collisionInfo)
        : object1(object1), object2(object2), collisionInfo(collisionInfo) { }
};

#endif //COLLISIONEVENT_H
