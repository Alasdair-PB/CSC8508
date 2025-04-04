#include "ToolsBar.h"
#include "IComponent.h"
#include <iostream>
#include <tuple>
#include <string>
#include "ComponentManager.h"
#include "GameWorld.h"
#include "EditorWindowManager.h"
#include "../EditorGame.h"

ToolsBar::ToolsBar() : editorManager(EditorWindowManager::Instance()), gameWorld(GameWorld::Instance())
{
	InitTools();
}

ToolsBar::~ToolsBar() {}

void ToolsBar::OnSetFocus(GameObject* focus){}
void ToolsBar::OnRenderFocus(GameObject* focus) {}
void ToolsBar::OnFocusEnd() {}
void ToolsBar::OnInit() {}

GameObject* ToolsBar::NewGameObject(GameObject* focus) {
	Vector3 position = focus ? focus->GetTransform().GetPosition() : Vector3(0, 0, 0);
	switch (primitive) {
	case Empty: {
		return new GameObject();
		break;
	}
	case Cube: {
		return EditorGame::GetInstance()->AddCubeToWorld(position, Vector3(0.5f, 0.5f, 0.5f));
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
	window = new UIElementsGroup(
		ImVec2(0.2f, 0.3f),
		ImVec2(0.1f, 0.5f),
		1.0f,
		"Tools",
		0.0f,
		ImGuiWindowFlags_NoResize);

	std::string* fileName = editorManager.GetFileName();
	GameObject** focus = editorManager.GetFocus();

	PushSetPrimitive();
	window->PushStatelessInputFieldElement("file", fileName);
	window->PushStatelessButtonElement(ImVec2(0.05f, 0.025f), "Save Pfab",
		[this, focus, fileName]() { 
			if ((*focus)) (*focus)->Save(editorManager.GetAssetPath(*fileName));
		});
	PushLoadPfab(fileName);
	window->PushStatelessButtonElement(ImVec2(0.05f, 0.025f), "Load World",
		[fileName, this]() {
			gameWorld.Load(editorManager.GetAssetPath(*fileName));
		});
	window->PushStatelessButtonElement(ImVec2(0.05f, 0.025f), "Save World",
		[fileName, this]() {
			gameWorld.Save(editorManager.GetAssetPath(*fileName));
		});
	window->PushStatelessButtonElement(ImVec2(0.05f, 0.025f), "Clear World",
		[this]() {
			editorManager.MarkWorldToClearWorld();
		});
	PushRemoveGameObject(focus);
	window->PushStatelessButtonElement(ImVec2(0.05f, 0.025f), "Add GameObject",
		[this, focus]() {
			editorManager.GetFileName();
			GameObject* loaded = NewGameObject(focus ? (*focus) : nullptr);
			gameWorld.AddGameObject(loaded);
			editorManager.SetFocus(loaded);
		});
	PushAddParent(focus);
	PushAddChild(focus);
	PushFocusParent(focus);
}

void ToolsBar::PushLoadPfab(std::string* fileName) {
	window->PushStatelessButtonElement(ImVec2(0.05f, 0.025f), "Load PFab",
		[fileName, this]() {
			std::cout << editorManager.GetAssetPath(*fileName) << std::endl;
			GameObject* loaded = new GameObject();
			loaded->Load(editorManager.GetAssetPath(*fileName));
			gameWorld.AddGameObject(loaded);
			editorManager.SetFocus(loaded);
		});
}

void ToolsBar::PushFocusParent(GameObject** focus) {
	window->PushStatelessButtonElement(ImVec2(0.05f, 0.025f), "Focus to Parent",
		[this, focus]() {
			if (!(*focus)) return;
			if ((*focus)->HasParent()) {
				GameObject* parent = (*focus)->TryGetParent();
				editorManager.SetFocus(parent);
			}
		});
}

void ToolsBar::PushAddChild(GameObject** focus){
	window->PushStatelessButtonElement(ImVec2(0.05f, 0.025f), "Add Child",
		[this, focus]() {
			if (!(*focus)) return;
			gameWorld.RemoveGameObject(*focus);
			GameObject* loaded = NewGameObject(*focus);
			(*focus)->AddChild(loaded);
			gameWorld.AddGameObject(*focus);
			editorManager.SetFocus(loaded);
		});
}

void ToolsBar::PushAddParent(GameObject** focus) {
	window->PushStatelessButtonElement(ImVec2(0.05f, 0.025f), "Add Parent",
		[this, focus]() {
			GameObject* focusObj = (*focus);
			if (!focusObj) return;
			GameObject* loaded = NewGameObject(focusObj);
			GameObject* parent = focusObj->TryGetParent();
			loaded->AddChild(focusObj);

			if (parent) {
				parent->RemoveChild(focusObj);
				parent->AddChild(loaded);
				gameWorld.RemoveGameObject(focusObj);
				gameWorld.RemoveGameObject(parent);
				gameWorld.AddGameObject(parent);
			}
			else {
				gameWorld.RemoveGameObject(focusObj);
				gameWorld.AddGameObject(loaded);
			}
			editorManager.SetFocus(loaded);
		});
}

void ToolsBar::PushRemoveGameObject(GameObject** focus) {
	window->PushStatelessButtonElement(ImVec2(0.05f, 0.025f), "Remove Obj",
		[this, focus]() {
			if (!(*focus)) return;
			gameWorld.RemoveGameObject(*focus);
			editorManager.EndFocus();
		});
}

void ToolsBar::PushLoadChild(GameObject** focus, std::string* fileName) {
	window->PushStatelessButtonElement(ImVec2(0.05f, 0.025f), "Load Child",
		[this, focus, fileName]() {
			if (!(*focus)) return;
			GameObject* loaded = new GameObject();
			loaded->Load(editorManager.GetAssetPath(*fileName));
			(*focus)->AddChild(loaded);
			gameWorld.RemoveGameObject(*focus);
			gameWorld.AddGameObject(*focus);
		});
}

void ToolsBar::PushSetPrimitive() {
	std::vector<std::pair<int*, std::string>> enumOptions = {
		{reinterpret_cast<int*>(&primitive), "Cube"},
		{reinterpret_cast<int*>(&primitive), "Sphere"},
		{reinterpret_cast<int*>(&primitive), "Empty"}
	};
	window->PushEnumElement("Component to add", enumOptions);
}

const static std::string folderPath = ASSETROOTLOCATION;
