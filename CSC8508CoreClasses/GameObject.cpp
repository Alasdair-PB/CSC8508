//
// Contributors: Alasdair
//
#include "IComponent.h"
#include "GameObject.h"
#include "EventManager.h"
#include "MaterialManager.h"
#include "RenderObject.h"
#include <unordered_map>
#include <unordered_set>
#include <typeindex>

#include <queue>
using namespace NCL::CSC8508;

AddComponentEvent::AddComponentEvent(GameObject& gameObject, size_t entry) : gameObject(gameObject), entry(entry) {}
GameObject& AddComponentEvent::GetGameObject() { return gameObject; }
size_t AddComponentEvent::GetEntry() { return entry; }

GameObject::GameObject(const bool newIsStatic): isStatic(newIsStatic), parent(nullptr) {
	worldID = -1;
	isEnabled = true;
	layerID = Layers::LayerID::Default;
	tags.push_back(Tags::Tag::Default);
	renderObject = nullptr;
	components = vector<IComponent*>();
	vector<Layers::LayerID> ignoreLayers = vector<Layers::LayerID>();
}

GameObject::~GameObject() {
	if (renderObject)
		delete renderObject;
}

struct GameObject::GameObjDataStruct : public ISerializedData {
	GameObjDataStruct() : isEnabled(true), orientation(Quaternion()), position(Vector3()), scale(Vector3(1,1,1)), colour(Vector4()),
		meshPointer(0), texturePointer(0), shaderPointer(0), name(""), layerId(Layers::Default) {}

	GameObjDataStruct(
		bool isEnabled, 
		Quaternion orientation, 
		Vector3 position, 
		Vector3 scale, 
		Vector4 colour, 
		size_t meshPointer, 
		size_t texturePointer, 
		size_t shaderPointer, 
		Layers::LayerID layerId, 
		vector<Tags::Tag> tags,
		std::string name):

		isEnabled(isEnabled), 
		orientation(orientation), 
		position(position), 
		scale(scale), 
		colour(colour), 
		texturePointer(texturePointer), 
		meshPointer(meshPointer), 
		shaderPointer(shaderPointer), 
		layerId(layerId),
		tags(tags), 
		name(name)  {}

	bool isEnabled;
	Quaternion orientation;
	Vector3 position;
	Vector3 scale;
	Vector4 colour;

	size_t meshPointer;
	size_t texturePointer;
	size_t shaderPointer;
	std::string name;

	std::vector<size_t> childrenPointers;
	std::vector<Tags::Tag> tags;

	Layers::LayerID layerId;
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
			SERIALIZED_FIELD(GameObjDataStruct, name),
			SERIALIZED_FIELD(GameObjDataStruct, shaderPointer),
			SERIALIZED_FIELD(GameObjDataStruct, childrenPointers),
			SERIALIZED_FIELD(GameObjDataStruct, tags),
			SERIALIZED_FIELD(GameObjDataStruct, layerId),
			SERIALIZED_FIELD(GameObjDataStruct, componentPointers)
		);
	}
};


void GameObject::LoadClean(GameObjDataStruct& loadedSaveData, std::string assetPath) {
	for (int i = 0; i < loadedSaveData.componentPointers.size(); i++) {
		AddComponentEvent e = AddComponentEvent(*this, loadedSaveData.componentPointers[i].second);
		EventManager::Call(&e);
		if (!e.IsCancelled())
			components.back()->Load(assetPath, loadedSaveData.componentPointers[i].first);
	}
}

void GameObject::LoadInto(GameObjDataStruct& loadedSaveData, std::string assetPath) {
	for (int i = 0; i < loadedSaveData.componentPointers.size(); i++) {
		if (i >= components.size()) break;
		components[i]->Load(assetPath, loadedSaveData.componentPointers[i].first);
	}
}

void GameObject::LoadGameObjectInstanceData(GameObjDataStruct loadedSaveData) {
	transform.SetOrientation(loadedSaveData.orientation);
	transform.SetPosition(loadedSaveData.position);
	transform.SetScale(loadedSaveData.scale);
	SetEnabled(loadedSaveData.isEnabled);
	tags = loadedSaveData.tags;
	layerID = loadedSaveData.layerId;
	Mesh* mesh = MaterialManager::GetMesh(loadedSaveData.meshPointer);
	Texture* basicTex = MaterialManager::GetTexture(loadedSaveData.texturePointer);
	Shader* basicShader = MaterialManager::GetShader(loadedSaveData.shaderPointer);

	#if EDITOR
	this->name = loadedSaveData.name;
	#endif

	if (mesh != nullptr && basicTex != nullptr) {
		renderObject = new RenderObject(&GetTransform(), mesh, basicTex, basicShader);
		renderObject->SetColour(loadedSaveData.colour);
	}
	else {
		renderObject = nullptr;
	}
}

void GameObject::Load(std::string assetPath, size_t allocationStart) {
	GameObjDataStruct loadedSaveData = ISerializedData::LoadISerializable<GameObjDataStruct>(assetPath, allocationStart);
	LoadGameObjectInstanceData(loadedSaveData);
	components.size() > 0 ? LoadInto(loadedSaveData, assetPath) : LoadClean(loadedSaveData, assetPath);
	LoadChildInstanceData(loadedSaveData, assetPath);
}

void GameObject::InitializeComponentMaps(
	std::unordered_map<IComponent*, int>& inDegree,
	std::unordered_map<IComponent*, std::unordered_set<std::type_index>>& dependencies,
	std::unordered_map<std::type_index, IComponent*>& typeToComponent)
{
	for (IComponent* component : components) {
		typeToComponent[std::type_index(typeid(*component))] = component;
		inDegree[component] = 0;
	}
}

void GameObject::BuildDependencyGraph(
	std::unordered_map<IComponent*, int>& inDegree,
	std::unordered_map<IComponent*, std::unordered_set<std::type_index>>& dependencies,
	const std::unordered_map<std::type_index, IComponent*>& typeToComponent)
{
	for (IComponent* component : components) {
		for (const std::type_index& depType : component->GetDependentTypes()) {
			if (typeToComponent.count(depType)) {
				IComponent* dependency = typeToComponent.at(depType);
				dependencies[dependency].insert(std::type_index(typeid(*component)));
				inDegree[component]++;
			}
		}
	}
}

bool GameObject::TopologicalSort(
	std::unordered_map<IComponent*, int>& inDegree,
	std::unordered_map<IComponent*, std::unordered_set<std::type_index>>& dependencies,
	const std::unordered_map<std::type_index, IComponent*>& typeToComponent,
	std::vector<IComponent*>& sortedComponents)
{
	std::queue<IComponent*> ready;
	for (IComponent* component : components) {
		if (inDegree[component] == 0)
			ready.push(component);
	}

	while (!ready.empty()) {
		IComponent* current = ready.front();
		ready.pop();
		sortedComponents.push_back(current);

		for (const std::type_index& depType : dependencies[current]) {
			IComponent* dependentComponent = typeToComponent.at(depType);
			if (--inDegree[dependentComponent] == 0)
				ready.push(dependentComponent);
		}
	}
	return sortedComponents.size() == components.size();
}

bool GameObject::HasNoDependencies(
	const std::unordered_map<IComponent*, std::unordered_set<std::type_index>>& dependencies) const
{
	for (const auto& [component, depSet] : dependencies) {
		if (!depSet.empty())
			return false;
	}
	return true;
}

void GameObject::OrderComponentsByDependencies() {
	std::unordered_map<IComponent*, int> inDegree;
	std::unordered_map<IComponent*, std::unordered_set<std::type_index>> dependencies;
	std::unordered_map<std::type_index, IComponent*> typeToComponent;

	InitializeComponentMaps(inDegree, dependencies, typeToComponent);
	BuildDependencyGraph(inDegree, dependencies, typeToComponent);
	if (HasNoDependencies(dependencies)) return;
	std::vector<IComponent*> sortedComponents;
	if (!TopologicalSort(inDegree, dependencies, typeToComponent, sortedComponents))
		std::cerr << "Error:: Cyclic dependency discovered in components" << std::endl;
	components = std::move(sortedComponents);
}

void GameObject::GetGameObjData(GameObjDataStruct& saveInfo) {
	std::string savedName = "Default";

	#if EDITOR
		savedName = this->name;
	#endif

	saveInfo = renderObject == nullptr ?
		GameObjDataStruct(isEnabled, transform.GetLocalOrientation(), transform.GetLocalPosition(), transform.GetLocalScale(),
			Vector4(), 0, 0, 0, layerID, tags, savedName) :
		GameObjDataStruct(isEnabled, transform.GetLocalOrientation(), transform.GetLocalPosition(), transform.GetLocalScale(),
			renderObject->GetColour(),
			MaterialManager::GetMeshPointer(renderObject->GetMesh()),
			MaterialManager::GetTexturePointer(renderObject->GetDefaultTexture()),
			MaterialManager::GetShaderPointer(renderObject->GetShader()), layerID, tags, savedName);
}

void GameObject::GetIComponentData(GameObjDataStruct& saveInfo, std::string assetPath, size_t* allocationStart) {
	for (IComponent* component : components) {
		std::string name = component->GetName();
		size_t id = SaveManager::MurmurHash3_64(name.c_str(), name.size());

		size_t nextMemoryLocation = component->Save(assetPath, allocationStart);
		saveInfo.componentPointers.push_back(std::make_pair(
			*allocationStart,
			id
		));
		*allocationStart = nextMemoryLocation;
	}
}

void GameObject::GetChildData(GameObjDataStruct& saveInfo, std::string assetPath, size_t* allocationStart) {
	for (GameObject* child : children) {
		size_t nextMemoryLocation = child->Save(assetPath, allocationStart);
		saveInfo.childrenPointers.push_back(*allocationStart);
		*allocationStart = nextMemoryLocation;
	}
}

void GameObject::LoadChildInstanceData(GameObjDataStruct& loadedSaveData, std::string assetPath){
	for (int i = 0; i < loadedSaveData.childrenPointers.size(); i++) {
		GameObject* object = new GameObject(isStatic);
		AddChild(object);
		object->Load(assetPath, loadedSaveData.childrenPointers[i]);
	}
}

void GameObject::CopyChildrenData(GameObject* gameObject)
{
	for (GameObject* child : children)
		gameObject->AddChild(child->CopyGameObject());
}

void GameObject::CopyIcomponentData(GameObject* gameObject)
{
	for (IComponent* component : components)
		component->CopyComponent(gameObject);
}

void GameObject::CopyInstanceData(GameObject* gameObject) {
	gameObject->GetTransform().SetPosition(transform.GetLocalPosition());
	gameObject->GetTransform().SetScale(transform.GetLocalScale());
	gameObject->GetTransform().SetOrientation(transform.GetLocalOrientation());
	gameObject->SetEnabled(isEnabled);
	gameObject->ClearTags();

	for (Tags::Tag tag:tags)
		gameObject->AddTag(tag);

	#if EDITOR
		gameObject->SetName(name);
	#endif

	if (renderObject) {
		RenderObject* copyRend = new RenderObject(&gameObject->GetTransform(),
			renderObject->GetMesh(),
			renderObject->GetDefaultTexture(),
			renderObject->GetShader());
		copyRend->SetColour(renderObject->GetColour());
		gameObject->SetRenderObject(copyRend);
	}
}

GameObject* GameObject::CopyGameObject() {
	OrderComponentsByDependencies();
	GameObject* copy = new GameObject(isStatic);
	CopyInstanceData(copy);
	CopyIcomponentData(copy);
	CopyChildrenData(copy);
	return copy;
}

size_t GameObject::Save(std::string assetPath, size_t* allocationStart)
{
	OrderComponentsByDependencies();
	bool clearMemory = false;
	if (allocationStart == nullptr) {
		allocationStart = new size_t(0);
		clearMemory = true;
	}

	GameObjDataStruct saveInfo;
	GetGameObjData(saveInfo);
	GetChildData(saveInfo, assetPath, allocationStart);
	GetIComponentData(saveInfo, assetPath, allocationStart);

	SaveManager::GameData saveData = ISerializedData::CreateGameData<GameObjDataStruct>(saveInfo);
	size_t nextMemoryLocation = SaveManager::SaveGameData(assetPath, saveData, allocationStart);

	if (clearMemory)
		delete allocationStart;
	return nextMemoryLocation;
}


bool GameObject::HasChild(GameObject* child) {
	return std::find(children.begin(), children.end(), child) != children.end();
}

bool GameObject::HasChildren() {
	return children.size() > 0;
}

void GameObject::AddChild(GameObject* child)
{
	if (child == nullptr || HasChild(child) || child->TryGetParent() == this) return;
	children.push_back(child);
	child->SetParent(this);
}

void GameObject::RemoveChild(GameObject* child) {
	if (child == nullptr || !HasChild(child)) return;
	children.erase(std::remove(children.begin(), children.end(), child), children.end());
}

GameObject* GameObject::TryGetParent() { return parent; }

void GameObject::SetParent(GameObject* newParent)
{
	if (newParent == nullptr) return;
	transform.SetParent(&newParent->GetTransform());
	this->parent = newParent;
}

bool GameObject::HasParent() { return parent == nullptr ? false : true; }

void GameObject::OperateOnChildren(std::function<void(GameObject*)> func) {
	for (GameObject* child : children)
		func(child);
}

void GameObject::DetatchComponent(IComponent* component){
	component->SetEnabled(false);
	components.erase(
		std::remove(components.begin(), components.end(), component),
		components.end()
	);
}

