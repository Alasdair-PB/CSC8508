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



/*struct IComponent::ComponentDataStruct : public ISerializedData {
	ComponentDataStruct() : enabled(1) {}
	ComponentDataStruct(bool enabled) : enabled(enabled) {}

	bool enabled;

	static auto GetSerializedFields() {
		return std::make_tuple(
			SERIALIZED_FIELD(ComponentDataStruct, enabled)
		);
	}
};

std::string IComponent::Save(std::string folderPath)
{
	int id = 6;
	std::string fileName = "game_data%" + std::to_string(id) + ".gdmt";
	ComponentDataStruct saveInfo(enabled);
	SaveManager::GameData saveData = ISerializedData::CreateGameData<ComponentDataStruct>(saveInfo);
	SaveManager::SaveGameData(fileName, saveData);
	return fileName;
}

void IComponent::Load(std::string folderPath, std::string name){

	ComponentDataStruct loadedSaveData = ISerializedData::LoadISerializable<ComponentDataStruct>(folderPath, name);
	std::cout << loadedSaveData.enabled << ": Component is enabled" << std::endl;
}*/