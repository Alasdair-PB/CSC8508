//
// Contributors: Alfie
//

#include "RoomPrefab.h"

struct RoomPrefab::RoomPrefabDataStruct : ISerializedData{
    std::vector<DoorLocation> doorLocations;
    std::vector<SpawnLocation> itemSpawnLocations;
    std::vector<SpawnLocation> enemySpawnLocations;

    RoomType roomType;
    int spawnProbability;

    RoomPrefabDataStruct() : 
        itemSpawnLocations(std::vector<SpawnLocation>()), 
        enemySpawnLocations(std::vector<SpawnLocation>()),
        doorLocations(std::vector<DoorLocation>()),
        roomType(Empty),
        spawnProbability(0) {}
    RoomPrefabDataStruct(
        std::vector<SpawnLocation> const& itemSpawnLocations, 
        std::vector<SpawnLocation> const& enemySpawnLocations,
        std::vector<DoorLocation> const& doorLocations,
        RoomType roomType,
        int spawnProbability)
        : 
        itemSpawnLocations(itemSpawnLocations), 
        enemySpawnLocations(enemySpawnLocations),
        doorLocations(doorLocations),
        roomType(roomType),
        spawnProbability(spawnProbability){}

    static auto GetSerializedFields() {
        return std::make_tuple(
            SERIALIZED_FIELD(RoomPrefabDataStruct, itemSpawnLocations),
            SERIALIZED_FIELD(RoomPrefabDataStruct, enemySpawnLocations),
            SERIALIZED_FIELD(RoomPrefabDataStruct, doorLocations),
            SERIALIZED_FIELD(RoomPrefabDataStruct, roomType),
            SERIALIZED_FIELD(RoomPrefabDataStruct, spawnProbability)
        );
    }
};

size_t RoomPrefab::Save(std::string const assetPath, size_t* allocationStart) {
    RoomPrefabDataStruct const saveInfo(itemSpawnLocations, enemySpawnLocations, doorLocations, roomType, spawnProbability);
    SaveManager::GameData const saveData = ISerializedData::CreateGameData<RoomPrefabDataStruct>(saveInfo);
    return SaveManager::SaveGameData(assetPath, saveData, allocationStart, true);
}

void RoomPrefab::Load(std::string const assetPath, size_t const allocationStart) {
    auto const loadedSaveData = ISerializedData::LoadISerializable<RoomPrefabDataStruct>(assetPath, allocationStart);
    itemSpawnLocations = loadedSaveData.itemSpawnLocations;
    doorLocations = loadedSaveData.doorLocations;
}

void RoomPrefab::PushIComponentElementsInspector(UIElementsGroup& elementsGroup, float scale)
{
    for (int i = 0; i < itemSpawnLocations.size(); i++) {
        elementsGroup.PushVectorElement(&itemSpawnLocations[i].location, scale, "Position");
        elementsGroup.PushFloatElement(&itemSpawnLocations[i].probability, scale, "Probability");
    }

    elementsGroup.PushStatelessButtonElement(ImVec2(scale, scale / 2), "Add new Item Spawn",
        [this]() {itemSpawnLocations.push_back(SpawnLocation());});

    for (int i = 0; i < doorLocations.size(); i++) {
        elementsGroup.PushVectorElement(&(doorLocations[i].dir), scale, "Dir");
        elementsGroup.PushVectorElement(&(doorLocations[i].pos), scale, "Pos");
    }

    elementsGroup.PushStatelessButtonElement(ImVec2(scale, scale / 2), "Add new Door",
        [this]() {doorLocations.push_back(DoorLocation());});
}
