#include "FileExplorer.h"
#include "IComponent.h"
#include <iostream>
#include <tuple>
#include <string>
#include "ComponentManager.h"
#include "GameWorld.h"
#include "EditorWindowManager.h"
#include "../EditorGame.h"

FileExplorer::FileExplorer() : editorManager(EditorWindowManager::Instance()), gameWorld(GameWorld::Instance())
{
	InitHierachy();
}

FileExplorer::~FileExplorer() = default;

void FileExplorer::OnSetFocus(GameObject* focus) {}
void FileExplorer::OnFocusEnd() {}
void FileExplorer::OnInit() {}

void FileExplorer::OnRenderFocus(GameObject* focus)
{
	window->ClearAllElements();
}

void FileExplorer::InitHierachy() {
	window = new UIElementsGroup(
		ImVec2(0.2f, 0.7f),
		ImVec2(0.5f, 0.3f),
		1.0f,
		"Explorer",
		0.0f
		);
}

void FileExplorer::PushHierarchyLabel(GameObject* object, std::string objChildIndent) {
	window->PushStatelessButtonElement(ImVec2(), objChildIndent + object->GetName(),
		[this, object]() { editorManager.SetFocus(object); }
	);
}