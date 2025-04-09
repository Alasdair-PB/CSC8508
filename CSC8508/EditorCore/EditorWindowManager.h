#ifndef EDITORMANAGER_H
#define EDITORMANAGER_H
#include "imgui.h"
#include "UIElementsGroup.h"
#include "GameObject.h"
#include "../Core/ComponentAssemblyDefiner.h"
#include "EditorWindow.h"
#include "RenderObject.h"

using namespace NCL;
using namespace CSC8508;
using namespace UI;


class EditorWindowManager {
public:
	EditorWindowManager();
	~EditorWindowManager();

	void SetFocusVisual(GameObject* object, bool focused) {
		RenderObject* renderObj = object->GetRenderObject();
		if (!renderObj) return;
		const Vector4 colour = focused ? Vector4(0, 1, 0, 1) : Vector4(1, 1, 1, 1);
		renderObj->SetColour(colour);
	}

	void SetFocus(GameObject* object) {
		if (!object) return;
		EndFocus();
		SetFocusVisual(object, true);
		focusElement = object;
		Vector3 posData = object->GetTransform().GetPosition();		
		SetVector(positionInfo, object->GetTransform().GetLocalPosition());
		SetVector(scaleInfo, object->GetTransform().GetLocalScale());
		SetQuaternion(orientationInfo, object->GetTransform().GetLocalOrientation());
		*isEnabled = object->IsEnabled();
		*name = focusElement->GetName();

		for (EditorWindow* window : windows)
			window->OnSetFocus(focusElement);
	}

	void RenderFocus() {
		if (clearWorld) ClearGameWorld();

		for (EditorWindow* window : windows)
			window->OnRenderFocus(focusElement);

		if (!focusElement) return;
		focusElement->GetTransform().SetPosition(*positionInfo);
		focusElement->GetTransform().SetScale(*scaleInfo);
		focusElement->GetTransform().SetOrientation(
			Quaternion(
				orientationInfo->x, orientationInfo->y, 
				orientationInfo->z, orientationInfo->w).Normalised());
		focusElement->SetEnabled(*isEnabled);
		focusElement->SetName(*name);
	}

	void EndFocus() {
		if (focusElement) SetFocusVisual(focusElement, false);
		focusElement = nullptr;
		*isEnabled = true;
		SetVector(positionInfo);
		SetVector(scaleInfo);
		SetQuaternion(orientationInfo);
	}

	static EditorWindowManager& Instance();
	void ClearGameWorld();
	GameObject** GetFocus() const { return focus; }
	Vector3* GetPositionInfo() const { return positionInfo;}
	Vector3* GetScaleInfo() const { return scaleInfo;}
	Vector4* GetOrientationInfo() const { return orientationInfo;}
	std::string* GetNameInfo() const { return name; }
	std::string* GetFileName() const { return saveDestination;}
	std::string* GetFolderPath() const { return folderPath; }
	std::string GetAssetPath(const std::string pfabName) const;
	std::string GetAssetRootDir();

	bool* GetEnabledInfo() const { return isEnabled; }
	void MarkWorldToClearWorld() { clearWorld = true; }
	void AddWindow(EditorWindow* window);
	void SetVector(Vector3* vector, Vector3 values = Vector3());
	void SetQuaternion(Vector4* quaternion, Quaternion values = Quaternion());

private:
	vector<EditorWindow*> windows;
	GameObject* focusElement;
	GameObject** focus;
	Vector3* positionInfo;
	Vector3* scaleInfo;
	Vector4* orientationInfo;

	bool* isEnabled;
	bool clearWorld;

	std::string* folderPath; 
	std::string* saveDestination;
	std::string* name;
};


#endif // EDITORMANAGER_H
