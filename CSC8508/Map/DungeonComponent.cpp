//
// Contributors: Alfie
//

#include "DungeonComponent.h"

void DungeonComponent::Generate() {

}


RoomComponent* DungeonComponent::TryGenerateRoom(RoomPrefab const& prefab, RoomPrefab::DoorLocation const& doorLocation) {

    // Make a new GameObject as a copy of the prefab's RoomObject
    auto attemptedRoom = new GameObject(*prefab.GetRoomObject());

    // Set the transform
    //attemptedRoom->GetTransform().SetPosition().

    return nullptr;
}

