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
