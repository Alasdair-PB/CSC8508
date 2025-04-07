#ifndef INSPECTOR_H
#define INSPECTOR_H
#include "imgui.h"
#include "UIElementsGroup.h"
#include "GameObject.h"
#include "../Core/ComponentAssemblyDefiner.h"
#include "EditorWindow.h"
#include "GameWorld.h"

using namespace NCL;
using namespace CSC8508;
using namespace UI;

class EditorWindowManager;

class Inspector : public EditorWindow {
public:
	Inspector();
	~Inspector();

	void OnSetFocus(GameObject* focus) override;
	void OnRenderFocus(GameObject* focus) override;
	void OnFocusEnd() override;
	void OnInit() override;
	std::string GetName() const override { return "Inspector"; }

private:
	EditorWindowManager& editorManager;
	GameWorld& gameWorld;
	int elementsCount;

	ComponentAssemblyDefiner::ComponentMapId mapId;
	Tags::Tag tagId;

	void PushAddComponentField(GameObject** focus);
	void PushTagField(GameObject* focus);
	void PushTagElements(GameObject* focus);
	void PushComponentInspector(GameObject* focus);

	void InitInspector();
};


#endif // INSPECTOR_H
