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

size_t GameObject::Save(std::string assetPath, size_t* allocationStart)
{
	bool clearMemory = false;
	if (allocationStart == nullptr) {
		allocationStart = new size_t(0);
		clearMemory = true;
	}

	GameObjDataStruct saveInfo(isEnabled);
	for (IComponent* component : components) {
		size_t nextMemoryLocation = component->Save(assetPath, allocationStart);		
		saveInfo.componentPointers.push_back(*allocationStart);
		*allocationStart = nextMemoryLocation;
	}
	SaveManager::GameData saveData = ISerializedData::CreateGameData<GameObjDataStruct>(saveInfo);
	size_t nextMemoryLocation = SaveManager::SaveGameData(assetPath, saveData, allocationStart);

	if (clearMemory)
		delete allocationStart;
	return nextMemoryLocation;
}
