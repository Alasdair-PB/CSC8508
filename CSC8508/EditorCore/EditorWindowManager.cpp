#include "EditorWindowManager.h"
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

EditorWindowManager::EditorWindowManager() : positionInfo(new Vector3()),
	scaleInfo(new Vector3()), orientationInfo(new Vector4()),
	isEnabled(new bool()),
	saveDestination(new std::string("Default.pfab")), 
	name(new std::string("")),
	clearWorld(false)
{}

EditorWindowManager& EditorWindowManager::Instance() {
	static EditorWindowManager instance;
	return instance;
}

void EditorWindowManager::AddWindow(EditorWindow* window) {
	if (!window) return;
	windows.push_back(window);
}

void EditorWindowManager::ClearGameWorld() {
	EndFocus();
	EditorGame::GetInstance()->DeleteSelectionObject();
	ComponentManager::CleanUp();
	GameWorld::Instance().ClearAndErase();
	clearWorld = false;
}

EditorWindowManager::~EditorWindowManager() {
	delete positionInfo;
	delete scaleInfo;
	delete orientationInfo;

	for (EditorWindow* window : windows) {
		delete window;
	}
	windows.clear();
}

GameObject* EditorWindowManager::NewGameObject() {
	Vector3 position = focus ? focus->GetTransform().GetPosition() : Vector3(0, 0, 0);
	switch (primitive) {
	case Empty: {
		return new GameObject();
		break;
	}
	case Cube: {
		return EditorGame::GetInstance()->AddCubeToWorld(position, Vector3(1,1,1));
		break;
	}
	case Sphere: {
		return EditorGame::GetInstance()->AddSphereToWorld(position, 1);
		break;
	}
	default: {
		break;
	}
	}
}

const static std::string folderPath = ASSETROOTLOCATION;

std::string EditorWindowManager::GetAssetPath(std::string pfabName) {
	return folderPath + "/Pfabs/" + pfabName;
}

void EditorWindowManager::SetVector(Vector3* vector, Vector3 values) {
	vector->x = values.x;
	vector->y = values.y;
	vector->z = values.z;
}

void EditorWindowManager::SetQuaternion(Vector4* quaternion, Quaternion values) {
	quaternion->x = values.x;
	quaternion->y = values.y;
	quaternion->z = values.z;
	quaternion->w = values.w;
}
