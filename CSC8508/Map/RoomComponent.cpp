//
// Contributors: Alfie
//

#include "RoomComponent.h"

#include "INetworkDeltaComponent.h" // Needed to use GameObject::AddComponent<>()

#include "CollisionDetection.h"
#include "RoomManager.h"

#include "../dungeon.h"

RoomComponent* RoomComponent::GenerateNew() {
    // 1: Pick a random prefab and create the game object for it
    auto* roomB = new GameObject();
    RoomPrefab* prefab = RoomManager::GetRandom();
    auto* component = roomB->AddComponent<RoomComponent>(prefab);

    // 2: Check for space against potential door positions
    for (RoomPrefab::DoorLocation aDoorLoc : this->prefab->GetDoorLocations()) {
        for (RoomPrefab::DoorLocation bDoorLoc : prefab->GetDoorLocations()) {

            Quaternion orientationDifference = Quaternion::VectorsToQuaternion(bDoorLoc.pos, aDoorLoc.pos);
            Transform& transformA = GetGameObject().GetTransform();
            Transform& transformB = roomB->GetTransform();
            transformB.SetOrientation(orientationDifference);
            transformB.SetPosition(
                transformA.GetPosition()
                + transformA.GetOrientation() * aDoorLoc.pos * transformA.GetScale()
                - transformB.GetOrientation() * bDoorLoc.pos * transformB.GetScale()
                );

            // Check if collides
            auto info = NCL::CollisionDetection::CollisionInfo();
             if (!NCL::CollisionDetection::ObjectIntersection(&GetGameObject(), GetDungeon(), info)) {

                 // Success (no collision) // TODO: Double check this is everything that needs doing
                 this->nextDoorRooms.push_back(component);
                 component->GetNextDoorRooms().push_back(this);
                 GetDungeon()->AddChild(roomB);
             }

            // TODO: Repeat if failed, try random doors and random prefabs
        }
    }

    // 3: Randomly select a valid position

    // 4: Spawn room in
    return nullptr;
}
