#ifndef EDITORMANAGER_H
#define EDITORMANAGER_H
#include "imgui.h"
#include "UIElementsGroup.h"
#include "GameObject.h"
#include "../ComponentAssemblyDefiner.h"
#include "EditorWindow.h"

using namespace NCL;
using namespace CSC8508;
using namespace UI;


class EditorWindowManager {
public:
	EditorWindowManager();
	~EditorWindowManager();

	void SetFocus(GameObject* object) {
		if (!object) return;
		focus = object;

		Vector3 posData = object->GetTransform().GetPosition();		
		SetVector(positionInfo, object->GetTransform().GetLocalPosition());
		SetVector(scaleInfo, object->GetTransform().GetLocalScale());
		SetQuaternion(orientationInfo, object->GetTransform().GetLocalOrientation());
		*isEnabled = object->IsEnabled();
		*name = focus->GetName();

		for (EditorWindow* window : windows)
			window->OnSetFocus(focus);
	}

	static EditorWindowManager& Instance();
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
		focus->SetName(*name);

		for (EditorWindow* window : windows)
			window->OnRenderFocus(focus);
	}

	GameObject* GetFocus() const { return focus; }
	Vector3* GetPositionInfo() const { return positionInfo;}
	Vector3* GetScaleInfo() const { return scaleInfo;}
	Vector4* GetOrientationInfo() const { return orientationInfo;}
	std::string* GetNameInfo() const { return name; }
	std::string* GetFileName() const { return saveDestination; }
	std::string GetFilePathInfo() const { return GetAssetPath(*saveDestination); }
	bool* GetEnabledInfo() const { return isEnabled; }

	void MarkWorldToClearWorld() { clearWorld = true; }
	void AddWindow(EditorWindow* window);
	void SetVector(Vector3* vector, Vector3 values = Vector3());
	void SetQuaternion(Vector4* quaternion, Quaternion values = Quaternion());

	void EndFocus() {
		focus = nullptr;
		*isEnabled = true;
		SetVector(positionInfo);
		SetVector(scaleInfo);
		SetQuaternion(orientationInfo);
	}

private:

	vector<EditorWindow*> windows;
	GameObject* focus;
	Vector3* positionInfo;
	Vector3* scaleInfo;
	Vector4* orientationInfo;

	bool* isEnabled;
	bool clearWorld;

	std::string* saveDestination;
	std::string* name;

	std::string GetAssetPath(const std::string pfabName) const;

};


#endif // EDITORMANAGER_H
