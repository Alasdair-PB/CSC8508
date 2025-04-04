#ifndef HIERARCHY_H
#define HIERARCHY_H
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

class Hierarchy : public EditorWindow {
public:
	Hierarchy();
	~Hierarchy();

	void OnSetFocus(GameObject* focus) override;
	void OnRenderFocus(GameObject* focus) override;
	void OnFocusEnd() override;
	void OnInit() override;
	std::string GetName() const override { return "Hierarchy"; }

private:
	EditorWindowManager& editorManager;
	GameWorld& gameWorld;
	void InitHierachy();
};


#endif // HIERARCHY_H
