#ifndef EDITORWINDOW_H
#define EDITORWINDOW_H
#include "imgui.h"
#include "UIElementsGroup.h"
#include "GameObject.h"
#include "../ComponentAssemblyDefiner.h"

using namespace NCL;
using namespace CSC8508;
using namespace UI;

class EditorWindow {
public:
	EditorWindow() : window(nullptr) {}
	~EditorWindow() {}

	virtual void OnSetFocus(GameObject* focus) {}
	virtual void OnRenderFocus(GameObject* focus) {}
	virtual void OnFocusEnd() {}
	virtual void OnInit() {}
	virtual std::string GetName() const { return "Default"; }
	UIElementsGroup* GetWindow() { return window; }

protected:
	UIElementsGroup* window;
};
#endif // EDITORWINDOW_H
