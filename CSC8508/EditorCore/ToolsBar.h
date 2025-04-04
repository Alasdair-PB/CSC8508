#ifndef TOOLS_H
#define TOOLS_H
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

class ToolsBar : public EditorWindow {
public:
	ToolsBar();
	~ToolsBar();

	GameObject* NewGameObject(GameObject* focus);
	void OnSetFocus(GameObject* focus) override;
	void OnRenderFocus(GameObject* focus) override;
	void OnFocusEnd() override;
	void OnInit() override;
	enum Primitives {Cube, Sphere, Empty};
	std::string GetName() const override { return "Tools"; }

private:
	Tags::Tag tagId;
	Primitives primitive;
	EditorWindowManager& editorManager;
	GameWorld& gameWorld;

	void PushLoadPfab(std::string* filePath);
	void PushAddChild(GameObject** focus);
	void PushSetPrimitive();
	void PushFocusParent(GameObject** focus);
	void PushAddParent(GameObject** focus);
	void PushLoadChild(GameObject** focus, std::string* filePath);
	void PushRemoveGameObject(GameObject** focus);

	void InitTools();
};


#endif // TOOLS_H
