#pragma once
#include "Controller.h"
#include "NavigationGrid.h"
#include "NavigationMesh.h"
#include "../UI/MainMenu.h"
#include "Math.h"
#include "GameTechRendererInterface.h"
#include "UISystem.h"
#ifdef USEVULKAN
#include "GameTechVulkanRenderer.h"
#endif
#include "PhysicsSystem.h"
#include "../Player/PlayerComponent.h"
#include "BoundsComponent.h"
#include <vector>
#include "SaveManager.h"
#include "ComponentAssemblyDefiner.h"
#include "UIElementsGroup.h"
#include "FramerateUI.h"
#include "MainMenuUI.h"
#include "../EditorCore/EditorWindowManager.h"
#include "../EditorCore/EditorCamera.h"

using std::vector;

namespace NCL {
	namespace CSC8508 {

		struct NetworkSpawnData
		{
			int objId;
			int ownId;
			bool clientOwned;
		};

		class EditorGame {
		public:
			EditorGame();
			~EditorGame();
			virtual void UpdateGame(float dt);
			void DeleteSelectionObject();
			static EditorGame* GetInstance() {return instance;}

			ComponentAssemblyDefiner* GetDefiner();
			GameObject* AddSphereToWorld(const Vector3& position, float radius, float inverseMass = 10.0f);
			GameObject* AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);

		protected:
			void InitialiseAssets();
			void InitWorld();
			void InitialiseGame();
			void SelectObject(BoundsComponent* newSelection);
			bool TrySelectObject();
			void SaveWorld(std::string assetPath);
			void LoadWorld(std::string assetPath);
			void UpdateUI();
			std::string GetAssetPath(std::string pfabName);

			GameObject* AddFloorToWorld(const Vector3& position);
			GameObject* AddNavMeshToWorld(std::string navMeshFilePath, std::string meshId, const Vector3& position, Vector3 dimensions);

			void SaveUnityNavMeshPrefab(std::string assetPath, std::string navMeshObPath, std::string navMeshNavPath);
			void  CalculateCubeTransformations(const std::vector<Vector3>& vertices, Vector3& position, Vector3& scale, Quaternion& rotation);
			std::vector<Vector3>  GetVertices(Mesh* navigationMesh, int i);

			bool inSelectionMode;
			inline static EditorGame* instance = nullptr;

			ComponentAssemblyDefiner* componentAssembly;
			PhysicsSystem* physics;
			GameWorld* world;
			Controller* controller;
			EditorWindowManager& windowManager;
			EditorCamera* editorCamera;

			BoundsComponent* selectionObject = nullptr;
			NavigationPath outPath;
			NavigationMesh* navMesh = nullptr;
			UI::UISystem* uiSystem;

#ifdef USEVULKAN
			GameTechVulkanRenderer*	renderer;
#else
			GameTechRendererInterface* renderer;
#endif
			
		};
	}
}

