#include "ToolsBar.h"
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

ToolsBar::ToolsBar() : positionInfo(new Vector3()),
	scaleInfo(new Vector3()), orientationInfo(new Vector4()),
	isEnabled(new bool()),
	saveDestination(new std::string("Default.pfab")), 
	name(new std::string("")),
	clearWorld(false)
{
	InitInspector();
	InitTools();
	InitHierachy();
}

ToolsBar::~ToolsBar() {
	delete inspectorBar;
	delete positionInfo;
	delete scaleInfo;
	delete orientationInfo;
}

GameObject* ToolsBar::NewGameObject() {
	Vector3 position = focus ? focus->GetTransform().GetPosition() : Vector3(0, 0, 0);
	switch (primitive) {
	case Empty: {
		return new GameObject();
		break;
	}
	case Cube: {
		return EditorGame::GetInstance()->AddCubeToWorld(position, Vector3(1,1,1));
		break;
	}
	case Sphere: {
		return EditorGame::GetInstance()->AddSphereToWorld(position, 1);
		break;
	}
	default: {
		break;
	}
	}
}

void ToolsBar::InitTools() {
	toolsBar = new UIElementsGroup(
		ImVec2(0.075f, 0.3f),
		ImVec2(0.1f, 0.5f),
		1.0f,
		"Tools",
		0.0f,
		ImGuiWindowFlags_NoResize);
	PushSetPrimitive();
	toolsBar->PushStatelessInputFieldElement("file", saveDestination);
	toolsBar->PushStatelessButtonElement(ImVec2(0.05f, 0.025f), "Save Pfab",
		[this]() { if (focus) focus->Save(GetAssetPath(*saveDestination)); });
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
	PushRemoveGameObject();
	toolsBar->PushStatelessButtonElement(ImVec2(0.05f, 0.025f), "Add GameObject",
		[this]() {
			GameObject* loaded = NewGameObject();
			GameWorld::Instance().AddGameObject(loaded);
			EndFocus();
			SetFocus(loaded);
		});
	PushAddParent();
	PushAddChild();
	PushFocusParent();
}

void ToolsBar::PushLoadPfab() {
	toolsBar->PushStatelessButtonElement(ImVec2(0.05f, 0.025f), "Load PFab",
		[this]() {
			GameObject* loaded = new GameObject();
			loaded->Load(GetAssetPath(*saveDestination));
			GameWorld::Instance().AddGameObject(loaded);
		});
}

void ToolsBar::PushFocusParent() {
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

void ToolsBar::PushAddParent() {
	toolsBar->PushStatelessButtonElement(ImVec2(0.05f, 0.025f), "Add Parent",
		[this]() {
			if (!focus) return;
			GameObject* loaded = NewGameObject();
			loaded->AddChild(focus);
			GameWorld::Instance().RemoveGameObject(focus);
			GameWorld::Instance().AddGameObject(loaded);
		});
}

void ToolsBar::PushRemoveGameObject() {
	toolsBar->PushStatelessButtonElement(ImVec2(0.05f, 0.025f), "Remove Obj",
		[this]() {
			if (!focus) return;
			GameWorld::Instance().RemoveGameObject(focus);
			EndFocus();
		});
}

void ToolsBar::PushAddChild() {
	toolsBar->PushStatelessButtonElement(ImVec2(0.05f, 0.025f), "Add Child",
		[this]() {
			if (!focus) return;
			GameObject* loaded = NewGameObject();
			focus->AddChild(loaded);
			GameWorld::Instance().RemoveGameObject(focus);
			GameWorld::Instance().AddGameObject(focus);
			EndFocus();
			SetFocus(loaded);
		});
}

void ToolsBar::PushLoadChild() {
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

void ToolsBar::PushSetPrimitive() {
	std::vector<std::pair<int*, std::string>> enumOptions = {
		{reinterpret_cast<int*>(&primitive), "Cube"},
		{reinterpret_cast<int*>(&primitive), "Sphere"},
		{reinterpret_cast<int*>(&primitive), "Empty"}
	};
	toolsBar->PushEnumElement("Component to add", enumOptions);
}

const static std::string folderPath = ASSETROOTLOCATION;

std::string ToolsBar::GetAssetPath(std::string pfabName) {
	return folderPath + "/Pfabs/" + pfabName;
}
