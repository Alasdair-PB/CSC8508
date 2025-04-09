//
// Contributors: Alfie
//

#include "RoomComponent.h"
#include <random>
#include "CollisionDetection.h"
#include "DoorLocation.h"
#include "RoomManager.h"
#include "../../CSC8508CoreClasses/Util.cpp"

RoomComponent::RoomComponent(GameObject& gameObject, RoomPrefab* prefab) : 
    IComponent(gameObject), prefab(prefab) {
}

void RoomComponent::SetTransform(const Transform& transformA, Transform& transformB, const Quaternion orientationDifference,
    const DoorLocation aDoorLoc, const DoorLocation bDoorLoc) {
    transformB.SetOrientation(orientationDifference * transformA.GetOrientation());
    transformB.SetPosition(
        transformA.GetPosition()
        + transformA.GetOrientation() * (transformA.GetScale() * aDoorLoc.pos)
        - transformB.GetOrientation() * (transformB.GetScale() * bDoorLoc.pos)
    );
}

bool RoomComponent::TryGenerateNewRoom(RoomComponent& roomB) {

    // Randomly order door locations
    auto const aDoorLocations = Util::RandomiseVector(this->prefab->GetDoorLocations());
    auto const bDoorLocations = Util::RandomiseVector(roomB.GetPrefab().GetDoorLocations());

    // Keep checking each combination until it finds a valid room
    Transform const& transformA = this->GetGameObject().GetTransform();
    Transform& transformB = roomB.GetGameObject().GetTransform();

    for (const DoorLocation aDoorLoc : aDoorLocations) {
        for (const DoorLocation bDoorLoc : bDoorLocations) {
            Quaternion orientationDifference = Quaternion::VectorsToQuaternion(bDoorLoc.dir, -aDoorLoc.dir);
            // Enforce flipping around the Y axis (no tilting the rooms)
            if (fabs(orientationDifference.x) >= FLT_EPSILON || fabs(orientationDifference.z) >= FLT_EPSILON) continue;
            SetTransform(transformA, transformB, orientationDifference, aDoorLoc, bDoorLoc);
            // Check if roomB's GameObject collides with any other object in the dungeon
            auto info = CollisionDetection::CollisionInfo();
            if (!CollisionDetection::ObjectIntersection(&roomB.GetGameObject(), GetDungeonGameObject(), info)) {
                this->nextDoorRooms.push_back(&roomB);
                roomB.GetNextDoorRooms().push_back(this);
                GetDungeonGameObject()->AddChild(&roomB.GetGameObject());
                return true;
            }
        }
    }
    // If no combination is valid, reset transform and return false
    transformB.SetOrientation(Quaternion());
    transformB.SetPosition(Vector3());
    return false;
}

