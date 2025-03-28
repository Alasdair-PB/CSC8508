#include "InventoryManagerComponent.h"

using namespace NCL;
using namespace CSC8508;


struct InventoryManagerComponent::InventoryManagerComponentDataStruct : public ISerializedData {
    InventoryManagerComponentDataStruct() : maxItemStorage(0) {}
    InventoryManagerComponentDataStruct(int maxItemStorage, float itemCarryOffset, float itemDropOffset)
        : maxItemStorage(maxItemStorage), itemCarryOffset(itemCarryOffset), itemDropOffset(itemDropOffset) {
    }

    int maxItemStorage;
    float itemCarryOffset;
    float itemDropOffset;
    float ite

    static auto GetSerializedFields() {
        return std::make_tuple(
            SERIALIZED_FIELD(InventoryManagerComponentDataStruct, maxItemStorage),
            SERIALIZED_FIELD(InventoryManagerComponentDataStruct, itemCarryOffset),
            SERIALIZED_FIELD(InventoryManagerComponentDataStruct, itemDropOffset)
        );
    }
};

size_t InventoryManagerComponent::Save(std::string assetPath, size_t* allocationStart) {
    InventoryManagerComponentDataStruct saveInfo(maxItemStorage, itemCarryOffset, itemDropOffset);
    SaveManager::GameData saveData = ISerializedData::CreateGameData<InventoryManagerComponentDataStruct>(saveInfo);
    return SaveManager::SaveGameData(assetPath, saveData, allocationStart, true);
}

void InventoryManagerComponent::Load(std::string assetPath, size_t allocationStart) {
    InventoryManagerComponentDataStruct loadedSaveData =
        ISerializedData::LoadISerializable<InventoryManagerComponentDataStruct>(assetPath, allocationStart);

    maxItemStorage = std::max(1, loadedSaveData.maxItemStorage);
    itemCarryOffset = loadedSaveData.itemCarryOffset;
    itemDropOffset = loadedSaveData.itemDropOffset;
}