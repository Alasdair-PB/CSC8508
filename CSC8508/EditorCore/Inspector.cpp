#include "Inspector.h"
#include "IComponent.h"
#include <iostream>
#include <tuple>
#include <string>
#include "ComponentManager.h"
#include "GameWorld.h"
#include "EditorWindowManager.h"
#include "../Core/EditorGame.h"
#include "MaterialManager.h"

Inspector::Inspector() : 
	editorManager(EditorWindowManager::Instance()), 
	gameWorld(GameWorld::Instance()), 
	elementsCount(0), meshIndex(new int()), textureIndex(new int())
{
	InitInspector();
}

Inspector::~Inspector() {
	delete meshIndex; 
	delete textureIndex;
}

void Inspector::OnSetFocus(GameObject* focus) {
	if (!focus) return;
	InitMaterial(focus);
	std::string* name = editorManager.GetNameInfo();
	bool* isEnabled = editorManager.GetEnabledInfo();
	Vector3* positionInfo = editorManager.GetPositionInfo();
	Vector3* scaleInfo = editorManager.GetScaleInfo();
	Vector4* orientationInfo = editorManager.GetOrientationInfo();

	window->PushStatelessInputFieldElement("GameObject:", name);
	window->PushToggle("Enabled:", isEnabled, 0.05f);
	window->PushVectorElement(positionInfo, 0.05f, "Position:");
	window->PushVectorElement(scaleInfo, 0.05f, "Scale");
	window->PushQuaternionElement(orientationInfo, 0.05f, "Orientation");
}

void Inspector::OnRenderFocus(GameObject* focus)
{ 
	if (!focus) return;
	window->RemoveElementsFromIndex(5);

	PushTagField(focus);
	PushLayerField(focus);
	PushRenderObject(focus);
	PushAddComponentField(focus);
	PushComponentInspector(focus); 
}

void Inspector::OnFocusEnd() { window->ClearAllElements(); }

void Inspector::OnInit() {}

void Inspector::InitInspector() {
	window = new UIElementsGroup(
		ImVec2(0.7f, 0.3f),
		ImVec2(0.3f, 0.5f),
		1.0f,
		"Inspector",
		0.0f,
		ImGuiWindowFlags_NoResize);
}

void Inspector::InitMaterial(GameObject* focus) {
	RenderObject* renderObject = focus->GetRenderObject();
	if (!renderObject) return;
	Mesh* currentMesh = renderObject->GetMesh();
	Texture* currentTexture = renderObject->GetDefaultTexture();

	if (currentMesh) {
		(*meshIndex) = 0;
		std::vector<std::pair<std::string, Mesh*>> sortedMeshList = GetMeshesSorted();
		for (int i = 0; i < sortedMeshList.size(); ++i) {
			if (currentMesh == sortedMeshList[i].second) {
				(*meshIndex) = i;
				break;
			}
		}
	}

	if (currentTexture) {
		(*textureIndex) = 0;
		std::vector<std::pair<std::string, Texture*>> sortedTextureList = GetTexturesSorted();
		for (int i = 0; i < sortedTextureList.size(); ++i) {
			if (currentTexture == sortedTextureList[i].second) {
				(*textureIndex) = i;
				break;
			}
		}
	}
}

void Inspector::PushAddComponentField(GameObject* focus) {
	std::vector<std::pair<int*, std::string>> enumOptions = {
		{reinterpret_cast<int*>(&mapId), "None"},
		{reinterpret_cast<int*>(&mapId), "Bounds"},
		{reinterpret_cast<int*>(&mapId), "Physics"},
		{reinterpret_cast<int*>(&mapId), "NavMesh"},
		{reinterpret_cast<int*>(&mapId), "Animation"},
		{reinterpret_cast<int*>(&mapId), "Damageable"},
		{reinterpret_cast<int*>(&mapId), "Room"}
	};
	window->PushEnumElement("Component to add", enumOptions);
	window->PushStatelessButtonElement(ImVec2(0.05f, 0.025f), "Add Component",
		[this, focus]() {
			if (!focus) return;
			EditorGame::GetInstance()->GetDefiner()->AddComponentFromEnum(mapId, *focus);
		});
}

void Inspector::PushLayerField(GameObject* focus) {
	Layers::LayerID* layerId = focus->GetLayerIDInfo();

	std::vector<std::pair<int*, std::string>> enumTagOptions = {
		{reinterpret_cast<int*>(layerId), "Default"},
		{reinterpret_cast<int*>(layerId), "Ignore_RayCast"},
		{reinterpret_cast<int*>(layerId), "UI"},
		{reinterpret_cast<int*>(layerId), "Player"},
		{reinterpret_cast<int*>(layerId), "Enemy"},
		{reinterpret_cast<int*>(layerId), "Ignore_Collisions"}
	};
	window->PushEnumElement("Layer to add", enumTagOptions);
}

std::vector<std::pair<std::string, Texture*>> Inspector::GetTexturesSorted() const {
	std::unordered_map<std::string, Texture*>& textureMappings = MaterialManager::GetTextureMap();
	std::vector<std::pair<std::string, Texture*>> sortedTextureList(textureMappings.begin(), textureMappings.end());
	std::sort(sortedTextureList.begin(), sortedTextureList.end(),
		[](const auto& a, const auto& b) {
			return a.first < b.first;
		});
	return sortedTextureList;
}

std::vector<std::pair<std::string, Mesh*>> Inspector::GetMeshesSorted() const {
	std::unordered_map<std::string, Mesh*>& meshMappings = MaterialManager::GetMeshMap();
	std::vector<std::pair<std::string, Mesh*>> sortedMeshList(meshMappings.begin(), meshMappings.end());
	std::sort(sortedMeshList.begin(), sortedMeshList.end(),
		[](const auto& a, const auto& b) {
			return a.first < b.first;
		});
	return sortedMeshList;
}

std::vector<std::pair<int*, std::string>> Inspector::GetTextureInfo(Texture** textureAtIndex,
	Texture* currentTexture, int* currentTextureIndex)
{
	std::vector<std::pair<int*, std::string>> textureOptions;
	std::vector<std::pair<std::string, Texture*>> sortedTextureList = GetTexturesSorted();

	for (int i = 0; i < sortedTextureList.size(); ++i) {
		if (i == (*textureIndex)) (*textureAtIndex) = sortedTextureList[i].second;
		if (currentTexture == sortedTextureList[i].second) (*currentTextureIndex) = i;
		textureOptions.emplace_back(reinterpret_cast<int*>(textureIndex), sortedTextureList[i].first);
	}
	return textureOptions;
}

std::vector<std::pair<int*, std::string>> Inspector::GetMeshInfo(Mesh** meshAtIndex, 
	Mesh* currentMesh, int* currentMeshIndex) 
{
	std::vector<std::pair<int*, std::string>> meshOptions;
	std::vector<std::pair<std::string, Mesh*>> sortedMeshList = GetMeshesSorted();

	for (int i = 0; i < sortedMeshList.size(); ++i) {
		if (i == (*meshIndex)) (*meshAtIndex) = sortedMeshList[i].second;
		if (currentMesh == sortedMeshList[i].second) (*currentMeshIndex) = i;
		meshOptions.emplace_back(reinterpret_cast<int*>(meshIndex), sortedMeshList[i].first);
	}
	return meshOptions;
}

void Inspector::PushMaterial(GameObject* focus, RenderObject* renderObject) {
	Mesh* currentMesh = renderObject->GetMesh();
	Texture* currentTexture = renderObject->GetDefaultTexture();

	if (!currentMesh && !currentTexture) return;

	Mesh* meshAtIndex = nullptr;
	Texture* textureAtIndex = nullptr;
	int currentMeshIndex = 0;
	int currentTextureIndex = 0;

	std::vector<std::pair<int*, std::string>> meshOptions = GetMeshInfo(&meshAtIndex, currentMesh, &currentMeshIndex);
	std::vector<std::pair<int*, std::string>> textureOptions = GetTextureInfo(&textureAtIndex, currentTexture, &currentTextureIndex);

	window->PushEnumElement("Texture", textureOptions);
	window->PushEnumElement("Mesh", meshOptions);
	CheckChangeMaterial(currentMeshIndex, meshAtIndex, currentTextureIndex, textureAtIndex, focus, renderObject);
}

void Inspector::CheckChangeMaterial(int currentMeshIndex, Mesh* meshAtIndex, int currentTextureIndex, Texture* textureAtIndex, GameObject* focus, RenderObject* renderObject)
{
	if ((*textureIndex) != currentTextureIndex || (*meshIndex) != currentMeshIndex) {
		RenderObject* newRenderObj = new RenderObject(&focus->GetTransform(),
			meshAtIndex,
			textureAtIndex,
			renderObject->GetShader());
		focus->SetRenderObject(newRenderObj);
		delete renderObject;
	}
}

void Inspector::PushRenderObject(GameObject* focus, RenderObject* renderObject) {
	window->PushStatelessButtonElement(ImVec2(0.05f, 0.025f), "Add Render Object",
		[this, focus]() {
			RenderObject* newRenderObj = new RenderObject(&focus->GetTransform(),
				MaterialManager::GetMesh("cube"),
				MaterialManager::GetTexture("basic"),
				MaterialManager::GetShader("basic"));
			focus->SetRenderObject(newRenderObj);
		});
}

void Inspector::PushRenderObject(GameObject* focus) {
	RenderObject* renderObject = focus->GetRenderObject();

	if (renderObject) PushMaterial(focus, renderObject);
	else PushRenderObject(focus, renderObject);
}

void Inspector::PushTagField(GameObject* focus) {
	vector<Tags::Tag>& tags = focus->GetTagInfo();
	for (Tags::Tag& tag : tags){
		std::vector<std::pair<int*, std::string>> enumTagOptions = {
			{reinterpret_cast<int*>(&tag), "Default"},
			{reinterpret_cast<int*>(&tag), "Player"},
			{reinterpret_cast<int*>(&tag), "Enemy"},
			{reinterpret_cast<int*>(&tag), "DropZone"},
			{reinterpret_cast<int*>(&tag), "CursorCast"},
			{reinterpret_cast<int*>(&tag), "Ground"},
			{reinterpret_cast<int*>(&tag), "DepositZone"},
			{reinterpret_cast<int*>(&tag), "Exit"}
		};
		window->PushEnumElement("", enumTagOptions);
	}
	window->PushStatelessButtonElement(ImVec2(0.05f, 0.025f), "Add Tag",
		[this, focus]() {
			if (!focus) return;
			(focus)->AddTag(Tags::Default);
		});

	window->PushStatelessButtonElement(ImVec2(0.05f, 0.025f), "Remove Tag",
		[this, focus]() {
			if (!focus) return;
			(focus)->RemoveTag();
		});
}

void Inspector::PushComponentInspector(GameObject* focus) {
	for (IComponent* component : (focus)->GetAllComponents()) {
		component->IComponent::PushIComponentElementsInspector(*window, 0.05f);
		component->PushIComponentElementsInspector(*window, 0.05f);

		window->PushStatelessButtonElement(ImVec2(0.05f, 0.025f), "Remove Component", 
			[focus, component]() {focus->DetatchComponent(component); });
	}
}