#include "Inspector.h"
#include "IComponent.h"
#include <iostream>
#include <tuple>
#include <string>
#include "ComponentManager.h"
#include "GameWorld.h"
#include "EditorWindowManager.h"
#include "../EditorGame.h"

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

	PushTagElements(focus);
	PushTagField(focus);
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

void Inspector::PushTagElements(GameObject* focus) {
	window->PushStaticTextElement("Tags on Object");
	for (Tags::Tag tag : (focus)->GetTags()) {
		std::string tagName = "Unknown";
		switch (tag) {
		case Tags::Default: {
			tagName = "Default";
			break;
		}
		case Tags::Player: {
			tagName = "Player";
			break;
		}
		case  Tags::Enemy: {
			tagName = "Enemy";
			break;
		}
		case  Tags::DropZone: {
			tagName = "DropZone";
			break;
		}
		case  Tags::CursorCast: {
			tagName = "CursorCast";
			break;
		}
		case  Tags::Ground: {
			tagName = "Ground";
			break;
		}
		case  Tags::DepositZone: {
			tagName = "DepositZone";
			break;
		}
		case  Tags::Exit: {
			tagName = "Exit";
			break;
		}
		default: {
			break;
		}
		}
		window->PushStaticTextElement(tagName);
	}
}

void Inspector::PushTagField(GameObject* focus) {
	std::vector<std::pair<int*, std::string>> enumTagOptions = {
		{reinterpret_cast<int*>(&tagId), "Default"},
		{reinterpret_cast<int*>(&tagId), "Player"},
		{reinterpret_cast<int*>(&tagId), "Enemy"},
		{reinterpret_cast<int*>(&tagId), "DropZone"},
		{reinterpret_cast<int*>(&tagId), "CursorCast"},
		{reinterpret_cast<int*>(&tagId), "Ground"},
		{reinterpret_cast<int*>(&tagId), "DepositZone"},
		{reinterpret_cast<int*>(&tagId), "Exit"}
	};

	window->PushEnumElement("Tag to add", enumTagOptions);
	window->PushStatelessButtonElement(ImVec2(0.05f, 0.025f), "Add Tag",
		[this, focus]() {
			if (!focus) return;
			(focus)->SetTag(tagId);
		});
}

void Inspector::PushComponentInspector(GameObject* focus) {
	for (IComponent* component : (focus)->GetAllComponents())
		component->PushIComponentElementsInspector(*window, 0.05f);
}