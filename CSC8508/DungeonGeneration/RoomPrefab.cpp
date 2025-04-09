//
// Contributors: Alfie
//

#include "RoomPrefab.h"

struct RoomPrefab::RoomPrefabDataStruct : ISerializedData{
    std::vector<Vector3> possibleItemSpawnLocations;
    std::vector<DoorLocation> doorLocations;

    RoomPrefabDataStruct() : possibleItemSpawnLocations(std::vector<Vector3>()), doorLocations(std::vector<DoorLocation>()) {}
    RoomPrefabDataStruct(std::vector<Vector3> const& possibleItemSpawnLocations, std::vector<DoorLocation> const& doorLocations)
        : possibleItemSpawnLocations(possibleItemSpawnLocations), doorLocations(doorLocations) {}

    static auto GetSerializedFields() {
        return std::make_tuple(
            SERIALIZED_FIELD(RoomPrefabDataStruct, possibleItemSpawnLocations),
            SERIALIZED_FIELD(RoomPrefabDataStruct, doorLocations)
            );
    }
};

size_t RoomPrefab::Save(std::string const assetPath, size_t* allocationStart) {
    RoomPrefabDataStruct const saveInfo(possibleItemSpawnLocations, doorLocations);
    SaveManager::GameData const saveData = ISerializedData::CreateGameData<RoomPrefabDataStruct>(saveInfo);
    return SaveManager::SaveGameData(assetPath, saveData, allocationStart, true);
}

void RoomPrefab::Load(std::string const assetPath, size_t const allocationStart) {
    auto const loadedSaveData = ISerializedData::LoadISerializable<RoomPrefabDataStruct>(assetPath, allocationStart);
    possibleItemSpawnLocations = loadedSaveData.possibleItemSpawnLocations;
    doorLocations = loadedSaveData.doorLocations;
}

void RoomPrefab::PushIComponentElementsInspector(UIElementsGroup& elementsGroup, float scale)
{
    for (int i = 0; i < possibleItemSpawnLocations.size(); i++)
        elementsGroup.PushVectorElement(&possibleItemSpawnLocations[i], scale, "ItemSpawn");

    elementsGroup.PushStatelessButtonElement(ImVec2(scale, scale / 2), "Add new Item Spawn",
        [this]() {possibleItemSpawnLocations.push_back(Vector3());});

    for (int i = 0; i < doorLocations.size(); i++) {
        elementsGroup.PushVectorElement(&(doorLocations[i].dir), scale, "Dir");
        elementsGroup.PushVectorElement(&(doorLocations[i].pos), scale, "Pos");
    }

    elementsGroup.PushStatelessButtonElement(ImVec2(scale, scale / 2), "Add new Door",
        [this]() {doorLocations.push_back(DoorLocation());});
}
