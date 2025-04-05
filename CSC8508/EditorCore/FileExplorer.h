#ifndef FILEEXPLORE_H
#define FILEEXPLORE_H
#include "imgui.h"
#include "UIElementsGroup.h"
#include "GameObject.h"
#include "../ComponentAssemblyDefiner.h"
#include "EditorWindow.h"
#include "GameWorld.h"

using namespace NCL;
using namespace CSC8508;
using namespace UI;

class EditorWindowManager;

class FileExplorer : public EditorWindow {
public:
	FileExplorer();
	~FileExplorer();

	void OnSetFocus(GameObject* focus) override;
	void OnRenderFocus(GameObject* focus) override;
	void OnFocusEnd() override;
	void OnInit() override;
	std::string GetName() const override { return "FileExplorer"; }

private:
	const std::string childIndent = "	";
	void PushHierarchyLabel(GameObject* object, std::string objChildIndent);
	EditorWindowManager& editorManager;
	GameWorld& gameWorld;
	void InitHierachy();
};


#endif // FILEEXPLORE_H
