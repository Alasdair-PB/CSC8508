#ifndef INSPECTOR_H
#define INSPECTOR_H
#include "imgui.h"
#include "UIElementsGroup.h"
#include "GameObject.h"

using namespace NCL;
using namespace CSC8508;
using namespace UI;

class Inspector {
public:
	Inspector();
	~Inspector();

	void SetFocus(GameObject* object) {
		if (!object) return;
		focus = object;

		Vector3 posData = object->GetTransform().GetPosition();		
		transformInfo->x = posData.x;
		transformInfo->y = posData.y;
		transformInfo->z = posData.z;
	}

	void RenderFocus() {
		if (!focus) return;
		focus->GetTransform().SetPosition(*transformInfo);
	}

	void RenderIComponents();

	void EndFocus() {
		focus = nullptr;
		transformInfo->x = 0;
		transformInfo->y = 0;
		transformInfo->z = 0;
	}

	UIElementsGroup* inspectorBar;

private:
	GameObject* focus;
	Vector3* transformInfo;
	vector<IComponent*> componentsList;
};


#endif // INSPECTOR_H
