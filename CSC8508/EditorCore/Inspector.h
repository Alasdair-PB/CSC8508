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
		SetVector(positionInfo, object->GetTransform().GetLocalPosition());
		SetVector(scaleInfo, object->GetTransform().GetLocalScale());
		SetQuaternion(orientationInfo, object->GetTransform().GetLocalOrientation());
		*isEnabled = object->IsEnabled();
	}

	void RenderFocus() {
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

	void RenderIComponents();

	void EndFocus() {
		focus = nullptr;
		*isEnabled = true;
		SetVector(positionInfo);
		SetVector(scaleInfo);
		SetQuaternion(orientationInfo);
	}

	UIElementsGroup* inspectorBar;
private:
	GameObject* focus;
	Vector3* positionInfo;
	Vector3* scaleInfo;
	Vector4* orientationInfo;
	bool* isEnabled;
	std::string* saveDestination;

	void SetVector(Vector3* vector, Vector3 values = Vector3()) {
		vector->x = values.x;
		vector->y = values.y;
		vector->z = values.z;
	}

	void SetQuaternion(Vector4* quaternion, Quaternion values = Quaternion()) {
		quaternion->x = values.x;
		quaternion->y = values.y;
		quaternion->z = values.z;
		quaternion->w = values.w;
	}

	template <typename T, typename... Args>
	void DebugSerializedFields(const T& instance, const std::tuple<Args...>& fields) {
		std::apply([&](const auto&... fieldTuple) {
			([&](const auto& entry) {
				auto fieldName = std::get<0>(entry);
				auto fieldPtr = std::get<1>(entry);

				std::cout << fieldName << std::endl;

				}(fieldTuple), ...);
			}, fields);
	}
};


#endif // INSPECTOR_H
