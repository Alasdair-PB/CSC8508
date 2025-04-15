#include "ItemComponent.h"

using namespace NCL;
using namespace CSC8508;

struct ItemComponent::ItemComponentDataStruct : public ISerializedData {
    ItemComponentDataStruct() : saleValue(0), itemWeight(0.0f) {}
    ItemComponentDataStruct(int saleValue, float itemWeight) : saleValue(saleValue), itemWeight(itemWeight) {}

    int saleValue;
    float itemWeight;

    static auto GetSerializedFields() {
        return std::make_tuple(
            SERIALIZED_FIELD(ItemComponentDataStruct, saleValue),
            SERIALIZED_FIELD(ItemComponentDataStruct, itemWeight)

        );
    }
};

size_t ItemComponent::Save(std::string assetPath, size_t* allocationStart) {
    ItemComponentDataStruct saveInfo(saleValue, itemWeight);
    SaveManager::GameData saveData = ISerializedData::CreateGameData<ItemComponentDataStruct>(saveInfo);
    return SaveManager::SaveGameData(assetPath, saveData, allocationStart, true);
}

void ItemComponent::Load(std::string assetPath, size_t allocationStart) {
    ItemComponentDataStruct loadedSaveData = ISerializedData::LoadISerializable<ItemComponentDataStruct>(assetPath, allocationStart);
    saleValue = std::max(0, loadedSaveData.saleValue);
    itemWeight = std::max(0.0f, loadedSaveData.itemWeight);
}