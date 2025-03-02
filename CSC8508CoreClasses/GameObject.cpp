//
// Contributors: Alasdair
//
#include "IComponent.h"
#include "GameObject.h"

using namespace NCL::CSC8508;

GameObject::GameObject(const bool newIsStatic): isStatic(newIsStatic), parent(nullptr) {
	worldID = -1;
	isEnabled = true;
	layerID = Layers::LayerID::Default;
	tag = Tags::Tag::Default;
	renderObject	= nullptr;
	components = vector<IComponent*>();
	vector<Layers::LayerID> ignoreLayers = vector<Layers::LayerID>();
}

GameObject::~GameObject() {
	delete renderObject;
}

struct GameObject::GameObjDataStruct : public ISerializedData {
	GameObjDataStruct() : isEnabled(1) {}
	GameObjDataStruct(bool isEnabled) : isEnabled(isEnabled) {}

	bool isEnabled;
	std::vector<size_t> componentPointers;

	static auto GetSerializedFields() {
		return std::make_tuple(
			SERIALIZED_FIELD(GameObjDataStruct, isEnabled),
			SERIALIZED_FIELD(GameObjDataStruct, componentPointers)
		);
	}
};

void GameObject::Load(std::string assetPath, size_t allocationStart) {

	GameObjDataStruct loadedSaveData = ISerializedData::LoadISerializable<GameObjDataStruct>(assetPath, allocationStart);
	for (int i = 0; i < loadedSaveData.componentPointers.size(); i++) {
		std::cout << loadedSaveData.componentPointers[i] << std::endl;
		if (i >= components.size()) break;
		components[i]->Load(assetPath, loadedSaveData.componentPointers[i]);
	}
	std::cout << loadedSaveData.isEnabled << std::endl;
}

size_t GameObject::Save(std::string assetPath, size_t allocationStart)
{
	GameObjDataStruct saveInfo(isEnabled);
	for (IComponent* component : components) {
		saveInfo.componentPointers.push_back(allocationStart);
		allocationStart = component->Save(assetPath, allocationStart);
	}
	SaveManager::GameData saveData = ISerializedData::CreateGameData<GameObjDataStruct>(saveInfo);
	SaveManager::SaveGameData(assetPath, saveData, allocationStart);
	return allocationStart;
}
