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
	ComponentAssemblyDefiner::ComponentMapId mapId;

	int elementsCount;
	int* meshIndex;

	void PushRenderObject(GameObject* focus);
	void PushAddComponentField(GameObject* focus);
	void PushTagField(GameObject* focus);
	void PushLayerField(GameObject* focus);
	void PushComponentInspector(GameObject* focus);
	void InitMaterial(GameObject* focus);
	void CheckChangeMesh(int currentMeshIndex, Mesh* meshAtIndex, GameObject* focus, RenderObject* renderObject);
	void PushMesh(GameObject* focus, RenderObject* renderObject);
	void PushRenderObject(GameObject* focus, RenderObject* renderObject);

	std::vector<std::pair<std::string, Mesh*>> GetMeshesSorted() const;
	std::vector<std::pair<int*, std::string>> GetMeshInfo(Mesh** meshAtIndex, Mesh* currentMesh, int* currentMeshIndex);
	void InitInspector();
};


#endif // INSPECTOR_H
