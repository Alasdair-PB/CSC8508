#include "Hierarchy.h"
#include "IComponent.h"
#include <iostream>
#include <tuple>
#include <string>
#include "ComponentManager.h"
#include "GameWorld.h"
#include "EditorWindowManager.h"
#include "../EditorGame.h"

Hierarchy::Hierarchy() : editorManager(EditorWindowManager::Instance()), gameWorld(GameWorld::Instance())
{
	InitHierachy();
}

Hierarchy::~Hierarchy() {}

void Hierarchy::OnSetFocus(GameObject* focus) {}
void Hierarchy::OnRenderFocus(GameObject* focus) {}
void Hierarchy::OnFocusEnd() {}
void Hierarchy::OnInit() {}

void Hierarchy::InitHierachy() {
	window = new UIElementsGroup(
		ImVec2(0.09f, 0.3f),
		ImVec2(0.1f, 0.5f),
		1.0f,
		"Hierarchy",
		0.0f,
		ImGuiWindowFlags_NoResize);
}