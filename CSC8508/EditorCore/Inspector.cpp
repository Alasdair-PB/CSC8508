#include "Inspector.h"
#include "IComponent.h"
#include <iostream>
#include <tuple>
#include <string>
#include "ComponentManager.h"
#include "GameWorld.h"
#include "EditorWindowManager.h"
#include "../Core/EditorGame.h"

Inspector::Inspector() : 
	editorManager(EditorWindowManager::Instance()), 
	gameWorld(GameWorld::Instance()), 
	elementsCount(0)
{
	InitInspector();
}

Inspector::~Inspector() {}

void Inspector::OnSetFocus(GameObject* focus) {}
void Inspector::OnRenderFocus(GameObject* focus)
{ 
	if (!focus) return;
	window->RemoveElementsFromIndex(7);

	PushTagField(focus);
	PushLayerField(focus);
	PushComponentInspector(focus); 
}

void Inspector::OnFocusEnd() {}
void Inspector::OnInit() {}

void Inspector::InitInspector() {
	window = new UIElementsGroup(
		ImVec2(0.7f, 0.3f),
		ImVec2(0.3f, 0.5f),
		1.0f,
		"Inspector",
		0.0f,
		ImGuiWindowFlags_NoResize);

	GameObject** focus = editorManager.GetFocus();
	std::string* name= editorManager.GetNameInfo();
	bool* isEnabled = editorManager.GetEnabledInfo();
	Vector3* positionInfo = editorManager.GetPositionInfo();
	Vector3* scaleInfo = editorManager.GetScaleInfo();
	Vector4* orientationInfo = editorManager.GetOrientationInfo();

	window->PushStatelessInputFieldElement("GameObject:", name);
	window->PushToggle("Enabled:", isEnabled, 0.05f);
	PushAddComponentField(focus);
	window->PushVectorElement(positionInfo, 0.05f, "Position:");
	window->PushVectorElement(scaleInfo, 0.05f, "Scale");
	window->PushQuaternionElement(orientationInfo, 0.05f, "Orientation");
}

void Inspector::PushAddComponentField(GameObject** focus) {
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
			if (!(*focus)) return;
			EditorGame::GetInstance()->GetDefiner()->AddComponentFromEnum(mapId, *(*focus));
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
		window->PushEnumElement("Tag to add", enumTagOptions);
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
	for (IComponent* component : (focus)->GetAllComponents())
		component->PushIComponentElementsInspector(*window, 0.05f);
}