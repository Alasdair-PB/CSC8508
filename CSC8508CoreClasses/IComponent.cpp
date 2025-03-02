//
// Contributors: Alasdair
//

#include "IComponent.h"

using namespace NCL::CSC8508;

GameObject& IComponent::GetGameObject() {
	return this->gameObject;
}

bool IComponent::IsEnabled() const {
	return this->enabled;
}

void IComponent::SetEnabled(bool enabled) {
	this->enabled = enabled;
}

struct IComponent::ComponentDataStruct : public ISerializedData {
	ComponentDataStruct() : enabled(1) {}
	ComponentDataStruct(bool enabled) : enabled(enabled) {}

	bool enabled;

	static auto GetSerializedFields() {
		return std::make_tuple(
			SERIALIZED_FIELD(ComponentDataStruct, enabled)
		);
	}
};

size_t IComponent::Save(std::string assetPath, size_t allocationStart)
{
	ComponentDataStruct saveInfo(enabled);
	SaveManager::GameData saveData = ISerializedData::CreateGameData<ComponentDataStruct>(saveInfo);
	return SaveManager::SaveGameData(assetPath, saveData, allocationStart);
}

void IComponent::Load(std::string assetPath, size_t allocationStart){

	ComponentDataStruct loadedSaveData = ISerializedData::LoadISerializable<ComponentDataStruct>(assetPath, allocationStart);
	std::cout << loadedSaveData.enabled << ": Component is enabled" << std::endl;
}