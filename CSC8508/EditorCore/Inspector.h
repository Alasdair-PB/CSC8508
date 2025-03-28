#ifndef INSPECTOR_H
#define INSPECTOR_H
#include "imgui.h"
#include "UIElementsGroup.h"
#include "GameObject.h"
#include "../ComponentAssemblyDefiner.h"

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
		SetVector(positionInfo, object->GetTransform().GetLocalPosition());
		SetVector(scaleInfo, object->GetTransform().GetLocalScale());
		SetQuaternion(orientationInfo, object->GetTransform().GetLocalOrientation());
		*isEnabled = object->IsEnabled();
	}
	
	void ClearGameWorld();

	void RenderFocus() {
		if (clearWorld) ClearGameWorld();
		if (!focus) return;
		focus->GetTransform().SetPosition(*positionInfo);
		focus->GetTransform().SetScale(*scaleInfo);
		focus->GetTransform().SetOrientation(
			Quaternion(
				orientationInfo->x, orientationInfo->y, 
				orientationInfo->z, orientationInfo->w).Normalised());
		focus->SetEnabled(*isEnabled);
		RenderIComponents();
	}

	GameObject* NewGameObject();

	void RenderIComponents();

	void EndFocus() {
		focus = nullptr;
		*isEnabled = true;
		SetVector(positionInfo);
		SetVector(scaleInfo);
		SetQuaternion(orientationInfo);
	}

	UIElementsGroup* inspectorBar;
	UIElementsGroup* toolsBar;

	enum Primitives {Cube, Sphere, Empty};

private:
	GameObject* focus;
	Vector3* positionInfo;
	Vector3* scaleInfo;
	Vector4* orientationInfo;
	ComponentAssemblyDefiner::ComponentMapId mapId;
	Tags::Tag tagId;
	Primitives primitive;
	bool* isEnabled;
	bool clearWorld;
	std::string* saveDestination;

	std::string GetAssetPath(std::string pfabName);
	void SetVector(Vector3* vector, Vector3 values = Vector3());
	void SetQuaternion(Vector4* quaternion, Quaternion values = Quaternion());
	void PushLoadPfab();
	void PushAddChild();
	void PushAddComponentField();
	void PushSetPrimitive();
	void PushTagField();
	void PushFocusParent();
	void PushAddParent();
	void PushLoadChild();
};


#endif // INSPECTOR_H
