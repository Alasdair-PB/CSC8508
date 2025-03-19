//
// Contributors: Alfie
//

#include "RoomComponent.h"
#include "RoomManager.h"

#include "../dungeon.h"

RoomComponent* RoomComponent::GenerateNew() {
    // 1: Pick a random prefab
    RoomPrefab* prefab = RoomManager::GetRandom();

    // 2: Check for space against potential door positions
    for (Vector3 dp : doorLocations) {
        // TODO: If is valid
    }

    // 3: Randomly select a valid position

    // 4: Spawn room in
    return nullptr;
}
