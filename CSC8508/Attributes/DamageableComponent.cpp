#include "DamageableComponent.h"
using namespace NCL::CSC8508;

struct DamageableComponent::DamageableComponentDataStruct : public ISerializedData {
	DamageableComponentDataStruct() : enabled(true), maxHealth(1), initHealth(1){}
	DamageableComponentDataStruct(bool enabled, int maxHealth, int initHealth) :
		enabled(enabled), maxHealth(maxHealth), initHealth(initHealth) {
	}

	bool enabled;
	int maxHealth;
	int initHealth;

	static auto GetSerializedFields() {
		return std::make_tuple(
			SERIALIZED_FIELD(DamageableComponentDataStruct, enabled),
			SERIALIZED_FIELD(DamageableComponentDataStruct, maxHealth),
			SERIALIZED_FIELD(DamageableComponentDataStruct, initHealth)
		);
	}
};

size_t DamageableComponent::Save(std::string assetPath, size_t* allocationStart)
{
	DamageableComponentDataStruct saveInfo;
	saveInfo = DamageableComponentDataStruct(IsEnabled(), maxHealth, health);
	SaveManager::GameData saveData = ISerializedData::CreateGameData<DamageableComponentDataStruct>(saveInfo);
	return SaveManager::SaveGameData(assetPath, saveData, allocationStart, false);
}

void DamageableComponent::Load(std::string assetPath, size_t allocationStart) {
	DamageableComponentDataStruct loadedSaveData = ISerializedData::LoadISerializable<DamageableComponentDataStruct>(assetPath, allocationStart);
	health = std::max(0, loadedSaveData.initHealth);
	maxHealth = std::max(1, loadedSaveData.maxHealth);
	health =std::min(health, maxHealth);
}
