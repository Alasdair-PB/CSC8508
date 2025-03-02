//
// Contributors: Alasdair
//
#include "IComponent.h"
#include "GameObject.h"

using namespace NCL::CSC8508;

GameObject::GameObject(const bool newIsStatic): isStatic(newIsStatic), parent(nullptr)	{
	worldID = -1;
	isEnabled = true;
	layerID = Layers::LayerID::Default;
	tag = Tags::Tag::Default;
	renderObject	= nullptr;
	components = vector<IComponent*>();
	vector<Layers::LayerID> ignoreLayers = vector<Layers::LayerID>();
}

GameObject::~GameObject()	{
	delete renderObject;
}

struct GameObject::GameObjDataStruct : public ISerializedData {
	GameObjDataStruct() : isEnabled(1) {}
	GameObjDataStruct(bool isEnabled) : isEnabled(isEnabled) {}

	bool isEnabled;
	std::vector<std::string> componentPointers;

	static auto GetSerializedFields() {
		return std::make_tuple(
			SERIALIZED_FIELD(GameObjDataStruct, isEnabled),
			SERIALIZED_FIELD(GameObjDataStruct, componentPointers)
		);
	}
};

void GameObject::Load(std::string folderPath, std::string name) {

	GameObjDataStruct loadedSaveData = ISerializedData::LoadISerializable<GameObjDataStruct>(folderPath, name);
	for (int i = 0; i < loadedSaveData.componentPointers.size(); i++) {
		std::cout << loadedSaveData.componentPointers[i] << std::endl;
		if (i >= components.size()) break;
		components[i]->Load(folderPath, loadedSaveData.componentPointers[i]);
	}
	std::cout << loadedSaveData.isEnabled << std::endl;
}

std::string GameObject::Save(std::string folderPath) 
{
	int id = 19;
	std::string fileName = "game_data%" + std::to_string(id) + ".gdmt";
	GameObjDataStruct saveInfo(isEnabled);

	for (IComponent* component : components)
		saveInfo.componentPointers.push_back(component->Save(folderPath));

	SaveManager::GameData saveData = ISerializedData::CreateGameData<GameObjDataStruct>(saveInfo);
	SaveManager::SaveGameData(fileName, saveData);
	return fileName;
}
