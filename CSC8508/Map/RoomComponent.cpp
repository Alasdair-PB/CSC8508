﻿//
// Contributors: Alfie
//

#include "RoomComponent.h"

#include <random>

#include "CollisionDetection.h"
#include "DoorLocation.h"
#include "RoomManager.h"
#include "../../CSC8508CoreClasses/Util.cpp"

RoomComponent::RoomComponent(GameObject& gameObject, RoomPrefab* prefab) : IComponent(gameObject), prefab(prefab) {
    for (GameObject* c : prefab->GetGameObject().GetChildren()) this->GetGameObject().AddChild(c);
}

bool RoomComponent::TryGenerateNewRoom(RoomComponent& roomB) {

    // Randomly order door locations
    auto const aDoorLocations = Util::RandomiseVector(this->prefab->GetDoorLocations());
    auto const bDoorLocations = Util::RandomiseVector(roomB.GetPrefab().GetDoorLocations());

    // Keep checking each combination until it finds a valid room
    Transform const& transformA = this->GetGameObject().GetTransform();
    Transform& transformB = roomB.GetGameObject().GetTransform();

    for (DoorLocation aDoorLoc : aDoorLocations) {
        for (DoorLocation bDoorLoc : bDoorLocations) {

            // Put the roomB GameObject in the test position
            Quaternion orientationDifference = Quaternion::VectorsToQuaternion(bDoorLoc.dir, -aDoorLoc.dir);
            // Enforce flipping around the Y axis (no tilting the rooms)
            if (fabs(orientationDifference.x) >= FLT_EPSILON || fabs(orientationDifference.z) >= FLT_EPSILON) continue;
            transformB.SetOrientation(orientationDifference * transformA.GetOrientation());
            transformB.SetPosition(
                transformA.GetPosition()
                + transformA.GetOrientation() * aDoorLoc.pos * transformA.GetScale()
                - transformB.GetOrientation() * bDoorLoc.pos * transformB.GetScale()
                );

            // Check if roomB's GameObject collides with any other object in the dungeon
            auto info = CollisionDetection::CollisionInfo();
            if (!CollisionDetection::ObjectIntersection(&roomB.GetGameObject(), GetDungeonGameObject(), info)) {

                // Success (no collision)
                this->nextDoorRooms.push_back(&roomB);
                roomB.GetNextDoorRooms().push_back(this);
                GetDungeonGameObject()->AddChild(&roomB.GetGameObject());
                return true;
            }

            // Else repeat until valid placement found
        }
    }

    // If no combination is valid, reset transform and return false
    transformB.SetOrientation(Quaternion());
    transformB.SetPosition(Vector3());
    return false;
}

