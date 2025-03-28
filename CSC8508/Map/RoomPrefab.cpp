//
// Contributors: Alfie
//

#include "RoomPrefab.h"

size_t RoomPrefab::Save(std::string const assetPath, size_t* allocationStart) {
    RoomPrefabDataStruct const saveInfo(possibleItemSpawnLocations, possibleDoorLocations);
    SaveManager::GameData const saveData = ISerializedData::CreateGameData<RoomPrefabDataStruct>(saveInfo);
    return SaveManager::SaveGameData(assetPath, saveData, allocationStart, true);
}

void RoomPrefab::Load(std::string const assetPath, size_t const allocationStart) {
    auto const loadedSaveData = ISerializedData::LoadISerializable<RoomPrefabDataStruct>(assetPath, allocationStart);
    possibleItemSpawnLocations = loadedSaveData.possibleItemSpawnLocations;
    possibleDoorLocations = loadedSaveData.possibleDoorLocations;
}

void RoomPrefab::PushIComponentElementsInspector(UIElementsGroup& elementsGroup, float scale)
{
    for (int i = 0; i < possibleItemSpawnLocations.size(); i++)
        elementsGroup.PushVectorElement(&possibleItemSpawnLocations[i], scale, "ItemSpawn");

    elementsGroup.PushStatelessButtonElement(ImVec2(scale, scale / 2), "Add new Item Spawn",
        [this]() {possibleItemSpawnLocations.push_back(Vector3());});

    for (int i = 0; i < possibleDoorLocations.size(); i++) {
        elementsGroup.PushVectorElement(&(possibleDoorLocations[i].dir), scale, "Dir");
        elementsGroup.PushVectorElement(&(possibleDoorLocations[i].pos), scale, "Pos");
    }

    elementsGroup.PushStatelessButtonElement(ImVec2(scale, scale / 2), "Add new Door",
        [this]() {possibleDoorLocations.push_back(DoorLocation());});
}
