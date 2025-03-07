//
// Contributors: Alasdair
//
#include "IComponent.h"
#include "GameObject.h"
#include "EventManager.h"
#include "RenderObject.h"

using namespace NCL::CSC8508;

AddComponentEvent::AddComponentEvent(GameObject& gameObject, size_t entry) : gameObject(gameObject), entry(entry) {}
GameObject& AddComponentEvent::GetGameObject() { return gameObject; }
size_t AddComponentEvent::GetEntry() { return entry; }

GameObject::GameObject(const bool newIsStatic): isStatic(newIsStatic), parent(nullptr) {
	worldID = -1;
	isEnabled = true;
	layerID = Layers::LayerID::Default;
	tag = Tags::Tag::Default;
	renderObject = nullptr;
	components = vector<IComponent*>();
	vector<Layers::LayerID> ignoreLayers = vector<Layers::LayerID>();
}

GameObject::~GameObject() {
	delete renderObject;
}

struct QuaternionSaveData {
public:
	float x;
	float y;
	float z;
	float w;
	QuaternionSaveData(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
	QuaternionSaveData() : x(0), y(0), z(0), w(0) {}
};

struct GameObject::GameObjDataStruct : public ISerializedData {
	GameObjDataStruct() : isEnabled(true), orientation(QuaternionSaveData()), position(Vector3()), scale(Vector3(1,1,1)), colour(Vector4()) {}
	GameObjDataStruct(bool isEnabled, QuaternionSaveData orientation, Vector3 position, Vector3 scale, Vector4 colour) :
		isEnabled(isEnabled), orientation(orientation), position(position), scale(scale), colour(colour){}

	bool isEnabled;
	QuaternionSaveData orientation;
	Vector3 position;
	Vector3 scale;
	Vector4 colour;
	std::vector<std::pair<size_t, size_t>> componentPointers;

	static auto GetSerializedFields() {
		return std::make_tuple(
			SERIALIZED_FIELD(GameObjDataStruct, isEnabled),
			SERIALIZED_FIELD(GameObjDataStruct, orientation),
			SERIALIZED_FIELD(GameObjDataStruct, position),
			SERIALIZED_FIELD(GameObjDataStruct, scale),
			SERIALIZED_FIELD(GameObjDataStruct, colour),
			SERIALIZED_FIELD(GameObjDataStruct, componentPointers)
		);
	}
};

void GameObject::LoadClean(GameObjDataStruct& loadedSaveData, std::string assetPath) {
	for (int i = 0; i < loadedSaveData.componentPointers.size(); i++) {
		std::cout << loadedSaveData.componentPointers[i].first << std::endl;
		std::cout << loadedSaveData.componentPointers[i].first << std::endl;

		AddComponentEvent e = AddComponentEvent(*this, loadedSaveData.componentPointers[i].second);
		EventManager::Call(&e);
		if (!e.IsCancelled())
			components.front()->Load(assetPath, loadedSaveData.componentPointers[i].first);
	}
}

void GameObject::LoadInto(GameObjDataStruct& loadedSaveData, std::string assetPath) {
	for (int i = 0; i < loadedSaveData.componentPointers.size(); i++) {
		std::cout << loadedSaveData.componentPointers[i].first << std::endl;
		std::cout << loadedSaveData.componentPointers[i].first << std::endl;

		if (i >= components.size()) break;
		components[i]->Load(assetPath, loadedSaveData.componentPointers[i].first);
	}
}

void GameObject::Load(std::string assetPath, size_t allocationStart) {
	GameObjDataStruct loadedSaveData = ISerializedData::LoadISerializable<GameObjDataStruct>(assetPath, allocationStart);
	components.size() > 0 ? LoadInto(loadedSaveData, assetPath) : LoadClean(loadedSaveData, assetPath);


	Quaternion orientation = Quaternion(loadedSaveData.orientation.x, loadedSaveData.orientation.y, loadedSaveData.orientation.z, loadedSaveData.orientation.w);
	transform.SetOrientation(orientation);
	transform.SetPosition(loadedSaveData.position);
	transform.SetScale(loadedSaveData.scale);
	SetEnabled(loadedSaveData.isEnabled);
	std::cout << loadedSaveData.isEnabled << std::endl;
}

size_t GameObject::Save(std::string assetPath, size_t* allocationStart)
{
	bool clearMemory = false;
	if (allocationStart == nullptr) {
		allocationStart = new size_t(0);
		clearMemory = true;
	}

	Vector4 colour = renderObject == nullptr ? Vector4(1,1,1,1) : renderObject->GetColour();

	Quaternion ori = transform.GetOrientation();
	QuaternionSaveData savedOritentation = QuaternionSaveData(ori.x, ori.y, ori.z, ori.w);
	GameObjDataStruct saveInfo(isEnabled, savedOritentation, transform.GetPosition(), transform.GetScale(), colour);

	for (IComponent* component : components) {
		size_t nextMemoryLocation = component->Save(assetPath, allocationStart);		
		saveInfo.componentPointers.push_back(std::make_pair(
			*allocationStart,
			SaveManager::MurmurHash3_64(typeid(*component).name(), std::strlen(typeid(*component).name()))
		));
		*allocationStart = nextMemoryLocation;
	}

	SaveManager::GameData saveData = ISerializedData::CreateGameData<GameObjDataStruct>(saveInfo);
	size_t nextMemoryLocation = SaveManager::SaveGameData(assetPath, saveData, allocationStart);

	if (clearMemory)
		delete allocationStart;
	return nextMemoryLocation;
}
