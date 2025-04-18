﻿//
// Contributors: Alfie & Alasdair
//

#include "RoomPrefab.h"
#include <random>
#include "CollisionDetection.h"
#include "DoorLocation.h"
#include "RoomManager.h"
#include "INetworkDeltaComponent.h"

struct RoomPrefab::RoomPrefabDataStruct : ISerializedData{
    std::vector<DoorLocation> doorLocations;
    std::vector<SpawnLocation> itemSpawnLocations;
    std::vector<SpawnLocation> enemySpawnLocations;

    RoomType roomType;
    float spawnProbability;

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
        float spawnProbability)
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

void RoomPrefab::CopyComponent(GameObject* gameObject) {
    RoomPrefab* component = gameObject->AddComponent<RoomPrefab>();
    component->SetEnabled(IsEnabled());
    component->doorLocations = doorLocations;
    component->enemySpawnLocations = enemySpawnLocations;
    component->itemSpawnLocations = itemSpawnLocations;
    component->spawnProbability = spawnProbability;
    component->roomType = roomType;
    component->nextDoorRooms = nextDoorRooms;
}

void RoomPrefab::Load(std::string const assetPath, size_t const allocationStart) {
    auto const loadedSaveData = ISerializedData::LoadISerializable<RoomPrefabDataStruct>(assetPath, allocationStart);
    itemSpawnLocations = loadedSaveData.itemSpawnLocations;
    enemySpawnLocations = loadedSaveData.enemySpawnLocations;
    doorLocations = loadedSaveData.doorLocations;

    roomType = loadedSaveData.roomType;
    spawnProbability = loadedSaveData.spawnProbability;
}

void RoomPrefab::PushIComponentElementsInspector(UIElementsGroup& elementsGroup, float scale)
{
    elementsGroup.PushStaticTextElement("Room Type");
    std::vector<std::pair<int*, std::string>> enumTagOptions = {
        {reinterpret_cast<int*>(&roomType), "Empty"},
        {reinterpret_cast<int*>(&roomType), "Bank"},
        {reinterpret_cast<int*>(&roomType), "Vender"},
        {reinterpret_cast<int*>(&roomType), "Exit"},
        {reinterpret_cast<int*>(&roomType), "Split"}
    };
    elementsGroup.PushEnumElement("", enumTagOptions);
    elementsGroup.PushFloatElement(&spawnProbability, scale, "Room spawn chance");

    elementsGroup.PushStaticTextElement("Item Spawns");
    for (int i = 0; i < itemSpawnLocations.size(); i++) {
        elementsGroup.PushVectorElement(&itemSpawnLocations[i].location, scale, "Position");
        elementsGroup.PushFloatElement(&itemSpawnLocations[i].probability, scale, "Probability");
    }    
    elementsGroup.PushStatelessButtonElement(ImVec2(scale, scale / 2), "Add Item Spawn",
        [this]() {itemSpawnLocations.push_back(SpawnLocation());});
    elementsGroup.PushStatelessButtonElement(ImVec2(scale, scale / 2), "Remove Item Spawn",
        [this]() { if (!itemSpawnLocations.empty()) itemSpawnLocations.pop_back(); });

    elementsGroup.PushStaticTextElement("Enemy Spawns");
    for (int i = 0; i < enemySpawnLocations.size(); i++) {
        elementsGroup.PushVectorElement(&enemySpawnLocations[i].location, scale, "Position");
        elementsGroup.PushFloatElement(&enemySpawnLocations[i].probability, scale, "Probability");
    }
    elementsGroup.PushStatelessButtonElement(ImVec2(scale, scale / 2), "Add Enemy Spawn",
        [this]() {enemySpawnLocations.push_back(SpawnLocation()); });
    elementsGroup.PushStatelessButtonElement(ImVec2(scale, scale / 2), "Remove Enemy Spawn",
        [this]() { if (!enemySpawnLocations.empty()) itemSpawnLocations.pop_back(); });

    elementsGroup.PushStaticTextElement("Door positions");
    for (int i = 0; i < doorLocations.size(); i++) {
        elementsGroup.PushVectorElement(&(doorLocations[i].dir), scale, "Dir");
        elementsGroup.PushVectorElement(&(doorLocations[i].pos), scale, "Pos");
    }
    elementsGroup.PushStatelessButtonElement(ImVec2(scale, scale / 2), "Add Door",
        [this]() {doorLocations.push_back(DoorLocation());});
    elementsGroup.PushStatelessButtonElement(ImVec2(scale, scale / 2), "Remove Door",
        [this]() { if (!doorLocations.empty()) doorLocations.pop_back(); });
}
