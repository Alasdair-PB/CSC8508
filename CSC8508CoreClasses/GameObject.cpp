//
// Contributors: Alasdair
//
#include "IComponent.h"
#include "GameObject.h"
#include "EventManager.h"
#include "MaterialManager.h"
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

struct GameObject::GameObjDataStruct : public ISerializedData {
	GameObjDataStruct() : isEnabled(true), orientation(Quaternion()), position(Vector3()), scale(Vector3(1,1,1)), colour(Vector4()),
		meshPointer(0), texturePointer(0), shaderPointer(0){}
	GameObjDataStruct(bool isEnabled, Quaternion orientation, Vector3 position, Vector3 scale, 
		Vector4 colour, size_t meshPointer, size_t texturePointer, size_t shaderPointer) :
		isEnabled(isEnabled), orientation(orientation), position(position), scale(scale), 
		colour(colour), texturePointer(texturePointer), meshPointer(meshPointer), shaderPointer(shaderPointer) {}

	bool isEnabled;
	Quaternion orientation;
	Vector3 position;
	Vector3 scale;
	Vector4 colour;
	size_t meshPointer;
	size_t texturePointer;
	size_t shaderPointer;

	std::vector<std::pair<size_t, size_t>> componentPointers;

	static auto GetSerializedFields() {
		return std::make_tuple(
			SERIALIZED_FIELD(GameObjDataStruct, isEnabled),
			SERIALIZED_FIELD(GameObjDataStruct, orientation),
			SERIALIZED_FIELD(GameObjDataStruct, position),
			SERIALIZED_FIELD(GameObjDataStruct, scale),
			SERIALIZED_FIELD(GameObjDataStruct, colour),
			SERIALIZED_FIELD(GameObjDataStruct, meshPointer),
			SERIALIZED_FIELD(GameObjDataStruct, texturePointer),
			SERIALIZED_FIELD(GameObjDataStruct, shaderPointer),
			SERIALIZED_FIELD(GameObjDataStruct, componentPointers)
		);
	}
};

void GameObject::LoadClean(GameObjDataStruct& loadedSaveData, std::string assetPath) {
	for (int i = 0; i < loadedSaveData.componentPointers.size(); i++) {
		std::cout << loadedSaveData.componentPointers[i].first << std::endl;
		AddComponentEvent e = AddComponentEvent(*this, loadedSaveData.componentPointers[i].second);
		EventManager::Call(&e);
		if (!e.IsCancelled())
			components.back()->Load(assetPath, loadedSaveData.componentPointers[i].first);
		std::cout << "new component added" << std::endl;

	}
}

void GameObject::LoadInto(GameObjDataStruct& loadedSaveData, std::string assetPath) {
	for (int i = 0; i < loadedSaveData.componentPointers.size(); i++) {
		std::cout << loadedSaveData.componentPointers[i].first << std::endl;
		if (i >= components.size()) break;
		components[i]->Load(assetPath, loadedSaveData.componentPointers[i].first);
	}
}

void GameObject::LoadGameObjectInstanceData(GameObjDataStruct loadedSaveData) {
	transform.SetOrientation(loadedSaveData.orientation);
	transform.SetPosition(loadedSaveData.position);
	transform.SetScale(loadedSaveData.scale);
	SetEnabled(loadedSaveData.isEnabled);

	Mesh* cubeMesh = MaterialManager::GetMesh(loadedSaveData.meshPointer);
	Texture* basicTex = MaterialManager::GetTexture(loadedSaveData.texturePointer);
	Shader* basicShader = MaterialManager::GetShader(loadedSaveData.shaderPointer);

	renderObject = new RenderObject(&GetTransform(), cubeMesh, basicTex, basicShader);
	renderObject->SetColour(loadedSaveData.colour);
	std::cout << loadedSaveData.isEnabled << std::endl;
}

void GameObject::Load(std::string assetPath, size_t allocationStart) {
	GameObjDataStruct loadedSaveData = ISerializedData::LoadISerializable<GameObjDataStruct>(assetPath, allocationStart);
	components.size() > 0 ? LoadInto(loadedSaveData, assetPath) : LoadClean(loadedSaveData, assetPath);
	LoadGameObjectInstanceData(loadedSaveData);
}

size_t GameObject::Save(std::string assetPath, size_t* allocationStart)
{
	bool clearMemory = false;
	if (allocationStart == nullptr) {
		allocationStart = new size_t(0);
		clearMemory = true;
	}
	GameObjDataStruct saveInfo;

	saveInfo = renderObject == nullptr ?
		
		GameObjDataStruct(isEnabled, transform.GetOrientation(), transform.GetPosition(), transform.GetScale(),
			Vector4(), 0, 0, 0) :
		GameObjDataStruct(isEnabled, transform.GetOrientation(), transform.GetPosition(), transform.GetScale(),
			renderObject->GetColour(), 
			MaterialManager::GetMeshPointer(renderObject->GetMesh()),
			MaterialManager::GetTexturePointer(renderObject->GetDefaultTexture()), 
			MaterialManager::GetShaderPointer(renderObject->GetShader()));

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

bool GameObject::HasChild(GameObject* child) {
	return std::find(children.begin(), children.end(), child) != children.end();
}

void GameObject::AddChild(GameObject* child)
{
	if (child == nullptr || HasChild(child)) return;
	children.push_back(child);
	child->SetParent(this);
}

void GameObject::RemoveChild(GameObject* child) {
	if (child == nullptr || HasChild(child)) return;
	children.erase(std::remove(children.begin(), children.end(), child), children.end());
}

GameObject* GameObject::TryGetParent() { return parent; }

void GameObject::SetParent(GameObject* newParent)
{
	if (newParent == nullptr) return;
	newParent->AddChild(this);
	this->parent = newParent;
}

bool GameObject::HasParent() { return parent == nullptr ? false : true; }