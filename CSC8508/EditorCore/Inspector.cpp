#include "Inspector.h"
#include "IComponent.h"
#include <iostream>
#include <tuple>
#include <string>
#include "ComponentManager.h"
#include "GameWorld.h"
#include "../EditorGame.h"

#ifndef ASSETROOTLOCATION
#define ASSETROOTLOCATION "../Assets/Pfabs"
#endif

Inspector::Inspector() : positionInfo(new Vector3()),
	scaleInfo(new Vector3()), orientationInfo(new Vector4()),
	isEnabled(new bool()),
	saveDestination(new std::string("Default.pfab")), clearWorld(false)
{
	inspectorBar = new UIElementsGroup(
		ImVec2(0.6f, 0.3f), 
		ImVec2(0.3f, 0.5f),
		1.0f, 
		"Inspector",
		0.0f, 
		ImGuiWindowFlags_NoResize);

	inspectorBar->PushToggle("GameObject:", isEnabled, 0.05f);
	inspectorBar->PushStatelessInputFieldElement("GameObject:", saveDestination);
	inspectorBar->PushStatelessButtonElement(ImVec2(0.05f, 0.025f), "Save Pfab", 
		[this]() { if (focus) focus->Save(GetAssetPath(*saveDestination));});

	inspectorBar->PushStatelessButtonElement(ImVec2(0.05f, 0.025f), "Clear World",
		[this]() { 
			clearWorld = true;
		});

	inspectorBar->PushVectorElement(positionInfo, 0.05f);
	inspectorBar->PushVectorElement(scaleInfo, 0.05f);
	inspectorBar->PushQuaternionElement(orientationInfo, 0.05f);
}

void Inspector::ClearGameWorld() {
	EndFocus();
	EditorGame::GetInstance()->DeleteSelectionObject();
	ComponentManager::CleanUp();
	GameWorld::Instance().ClearAndErase();
	clearWorld = false;
}

Inspector::~Inspector() { 
	delete inspectorBar; 
	delete positionInfo; 
	delete scaleInfo;
	delete orientationInfo;
}

const static std::string folderPath = ASSETROOTLOCATION;

std::string Inspector::GetAssetPath(std::string pfabName) {
	return folderPath + pfabName;
}

void Inspector::RenderIComponents() {
	if (!focus) return;
	inspectorBar->RemoveElementsFromIndex(6);

	for (IComponent* component : focus->GetAllComponents()) {
		component->PushIComponentElementsInspector(*inspectorBar, 0.05f);
	}
}

void Inspector::SetVector(Vector3* vector, Vector3 values) {
	vector->x = values.x;
	vector->y = values.y;
	vector->z = values.z;
}

void Inspector::SetQuaternion(Vector4* quaternion, Quaternion values) {
	quaternion->x = values.x;
	quaternion->y = values.y;
	quaternion->z = values.z;
	quaternion->w = values.w;
}
