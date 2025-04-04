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

Hierarchy::~Hierarchy() = default;

void Hierarchy::OnSetFocus(GameObject* focus) {}
void Hierarchy::OnFocusEnd() {}
void Hierarchy::OnInit() {}

void Hierarchy::OnRenderFocus(GameObject* focus)
{
	window->ClearAllElements();
	PushGameObjects(focus);
}

void Hierarchy::InitHierachy() {
	window = new UIElementsGroup(
		ImVec2(0.03f, 0.3f),
		ImVec2(0.1f, 0.5f),
		1.0f,
		"Hierarchy",
		0.0f,
		ImGuiWindowFlags_NoResize);
}

void Hierarchy::PushHierarchyLabel(GameObject* object, std::string objChildIndent) {
	window->PushStatelessButtonElement(ImVec2(), objChildIndent + object->GetName(),
		[this, object]() { editorManager.SetFocus(object); }
	);
}

void Hierarchy::PushChildren(GameObject* object, std::string objChildIndent) {
	if (object->HasChildren()) {
		for (GameObject* child : object->GetChildren()) {
			PushHierarchyLabel(child, objChildIndent);
			PushChildren(child, (objChildIndent + childIndent));
		}
	}
}

void Hierarchy::PushGameObjects(GameObject* focus) {
	std::string objectName;
	std::vector<GameObject*>::const_iterator first;
	std::vector<GameObject*>::const_iterator last;
	gameWorld.GetObjectIterators(first, last);

	for (auto it = first; it != last; ++it) {
		GameObject* object = (*it);
		if (object->HasParent()) continue;
		PushHierarchyLabel(object, "");
		PushChildren(object, childIndent);
	}
}