#include "ItemComponent.h"

using namespace NCL;
using namespace CSC8508;

struct ItemComponent::ItemComponentDataStruct : public ISerializedData {
    ItemComponentDataStruct() : saleValue(0) {}
    ItemComponentDataStruct(int saleValue) : saleValue(saleValue) {}

    int saleValue;

    static auto GetSerializedFields() {
        return std::make_tuple(
            SERIALIZED_FIELD(ItemComponentDataStruct, saleValue)
        );
    }
};

size_t ItemComponent::Save(std::string assetPath, size_t* allocationStart) {
    ItemComponentDataStruct saveInfo(saleValue);
    SaveManager::GameData saveData = ISerializedData::CreateGameData<ItemComponentDataStruct>(saveInfo);
    return SaveManager::SaveGameData(assetPath, saveData, allocationStart, true);
}

void ItemComponent::Load(std::string assetPath, size_t allocationStart) {
    ItemComponentDataStruct loadedSaveData = ISerializedData::LoadISerializable<ItemComponentDataStruct>(assetPath, allocationStart);
    saleValue = std::max(0, loadedSaveData.saleValue);
}