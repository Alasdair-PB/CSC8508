#include "Inspector.h"
#include "IComponent.h"
#include <iostream>
#include <tuple>
#include <string>
#include "ComponentManager.h"
#include "GameWorld.h"
#include "../EditorGame.h"

#ifndef ASSETROOTLOCATION
#define ASSETROOTLOCATION "../Assets/Pfabs"
#endif

Inspector::Inspector() : positionInfo(new Vector3()),
	scaleInfo(new Vector3()), orientationInfo(new Vector4()),
	isEnabled(new bool()),
	saveDestination(new std::string("Default.pfab")), clearWorld(false)
{
	inspectorBar = new UIElementsGroup(
		ImVec2(0.7f, 0.3f), 
		ImVec2(0.3f, 0.5f),
		1.0f, 
		"Inspector",
		0.0f, 
		ImGuiWindowFlags_NoResize);

	toolsBar = new UIElementsGroup(
		ImVec2(0.075f, 0.3f),
		ImVec2(0.1f, 0.5f),
		1.0f,
		"Tools",
		0.0f,
		ImGuiWindowFlags_NoResize);

	inspectorBar->PushToggle("GameObject:", isEnabled, 0.05f);
	toolsBar->PushStatelessInputFieldElement("file", saveDestination);
	toolsBar->PushStatelessButtonElement(ImVec2(0.05f, 0.025f), "Save Pfab",
		[this]() { if (focus) focus->Save(GetAssetPath(*saveDestination));});
	PushLoadPfab();
	toolsBar->PushStatelessButtonElement(ImVec2(0.05f, 0.025f), "Load World",
		[this]() {
			GameWorld::Instance().Load(GetAssetPath(*saveDestination));
		});
	toolsBar->PushStatelessButtonElement(ImVec2(0.05f, 0.025f), "Save World",
		[this]() {
			GameWorld::Instance().Save(GetAssetPath(*saveDestination));
		});
	toolsBar->PushStatelessButtonElement(ImVec2(0.05f, 0.025f), "Clear World",
		[this]() { 
			clearWorld = true;
		});

	toolsBar->PushStatelessButtonElement(ImVec2(0.05f, 0.025f), "Add GameObject",
		[this]() {
			GameObject* loaded = new GameObject();
			GameWorld::Instance().AddGameObject(loaded);
		});

	PushAddParent();
	PushAddChild();
	PushFocusParent();
	PushAddComponentField();

	inspectorBar->PushVectorElement(positionInfo, 0.05f, "Position:");
	inspectorBar->PushVectorElement(scaleInfo, 0.05f, "Scale");
	inspectorBar->PushQuaternionElement(orientationInfo, 0.05f, "Orientation");
}

void Inspector::PushLoadPfab() {
	toolsBar->PushStatelessButtonElement(ImVec2(0.05f, 0.025f), "Load PFab",
		[this]() {
			GameObject* loaded = new GameObject();
			loaded->Load(GetAssetPath(*saveDestination));
			GameWorld::Instance().AddGameObject(loaded);
		});
}

void Inspector::PushFocusParent() {
	toolsBar->PushStatelessButtonElement(ImVec2(0.05f, 0.025f), "Focus to Parent",
		[this]() {
			if (!focus) return;
			if (focus->HasParent()) {
				GameObject* parent = focus->TryGetParent();
				EndFocus();
				SetFocus(parent);
			}
		});
}

void Inspector::PushAddParent() {
	toolsBar->PushStatelessButtonElement(ImVec2(0.05f, 0.025f), "Add Parent",
		[this]() {
			if (!focus) return;
			GameObject* loaded = new GameObject();
			loaded->AddChild(focus);
			GameWorld::Instance().RemoveGameObject(focus);
			GameWorld::Instance().AddGameObject(loaded);
		});
}

void Inspector::PushAddChild() {
	toolsBar->PushStatelessButtonElement(ImVec2(0.05f, 0.025f), "Add Child",
		[this]() {
			if (!focus) return;
			GameObject* loaded = new GameObject();
			focus->AddChild(loaded);
			GameWorld::Instance().RemoveGameObject(focus);
			GameWorld::Instance().AddGameObject(focus);
		});
}

void Inspector::PushLoadChild() {
	toolsBar->PushStatelessButtonElement(ImVec2(0.05f, 0.025f), "Load Child",
		[this]() {
			if (!focus) return;
			GameObject* loaded = new GameObject();
			loaded->Load(GetAssetPath(*saveDestination));
			focus->AddChild(loaded);
			GameWorld::Instance().RemoveGameObject(focus);
			GameWorld::Instance().AddGameObject(focus);
		});
}

void Inspector::PushAddComponentField() {
	std::vector<std::pair<int*, std::string>> enumOptions = {
	{reinterpret_cast<int*>(&mapId), "None"},
	{reinterpret_cast<int*>(&mapId), "Bounds"},
	{reinterpret_cast<int*>(&mapId), "Physics"},
	{reinterpret_cast<int*>(&mapId), "NavMesh"},
	{reinterpret_cast<int*>(&mapId), "Animation"},
	{reinterpret_cast<int*>(&mapId), "Damageable"},
	{reinterpret_cast<int*>(&mapId), "Room"}

	};
	inspectorBar->PushEnumElement("Component to add", enumOptions);
	inspectorBar->PushStatelessButtonElement(ImVec2(0.05f, 0.025f), "Add Component",
		[this]() {
			if (!focus) return;
			EditorGame::GetInstance()->GetDefiner()->AddComponentFromEnum(mapId, *focus);
		});
}

void Inspector::PushTagField() {
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

	inspectorBar->PushEnumElement("Tag to add", enumTagOptions);
	inspectorBar->PushStatelessButtonElement(ImVec2(0.05f, 0.025f), "Add Tag",
		[this]() {
			if (!focus) return;
			focus->SetTag(tagId);
		});

	inspectorBar->PushStaticTextElement("Tags on Object");
	for (Tags::Tag tag : focus->GetTags()) {
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
		inspectorBar->PushStaticTextElement(tagName);
	}
}

void Inspector::ClearGameWorld() {
	EndFocus();
	EditorGame::GetInstance()->DeleteSelectionObject();
	ComponentManager::CleanUp();
	GameWorld::Instance().ClearAndErase();
	clearWorld = false;
}

Inspector::~Inspector() { 
	delete inspectorBar; 
	delete positionInfo; 
	delete scaleInfo;
	delete orientationInfo;
}

const static std::string folderPath = ASSETROOTLOCATION;

std::string Inspector::GetAssetPath(std::string pfabName) {
	return folderPath + "/Pfabs/" + pfabName;
}

void Inspector::RenderIComponents() {
	if (!focus) return;
	inspectorBar->RemoveElementsFromIndex(6);

	PushTagField();
	for (IComponent* component : focus->GetAllComponents())
		component->PushIComponentElementsInspector(*inspectorBar, 0.05f);
}

void Inspector::SetVector(Vector3* vector, Vector3 values) {
	vector->x = values.x;
	vector->y = values.y;
	vector->z = values.z;
}

void Inspector::SetQuaternion(Vector4* quaternion, Quaternion values) {
	quaternion->x = values.x;
	quaternion->y = values.y;
	quaternion->z = values.z;
	quaternion->w = values.w;
}
