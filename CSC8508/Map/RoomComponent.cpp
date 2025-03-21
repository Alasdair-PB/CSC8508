//
// Contributors: Alfie
//

#include "RoomComponent.h"
#include "RoomManager.h"

#include "../dungeon.h"

RoomComponent* RoomComponent::GenerateNew() {
    // 1: Pick a random prefab
    auto* roomB = new GameObject();
    RoomPrefab* prefab = RoomManager::GetRandom();

    // 2: Check for space against potential door positions
    for (RoomPrefab::DoorLocation dp : this->prefab->GetDoorLocations()) {
        for (RoomPrefab::DoorLocation newDoorLoc : prefab->GetDoorLocations()) {
            Quaternion quat = Quaternion::VectorsToQuaternion(newDoorLoc.pos, dp.pos);
            prefab->GetRoomObject()->GetTransform().SetOrientation(quat);
            prefab->GetRoomObject()->GetTransform().SetPosition(
                GetGameObject().GetTransform().GetPosition() // TODO: Not done
                );
        }
        // TODO: If is valid
    }

    // 3: Randomly select a valid position

    // 4: Spawn room in
    return nullptr;
}
