#pragma once
#include "Controller.h"
#include "NavigationGrid.h"
#include "NavigationMesh.h"
#include "Legacy/MainMenu.h"
#include "Math.h"
#include "GameTechRendererInterface.h"
#include "UISystem.h"
#ifdef USEVULKAN
#include "GameTechVulkanRenderer.h"
#endif
#include "PhysicsSystem.h"
#include "Legacy/PlayerComponent.h"
#include "BoundsComponent.h"
#include <vector>
#include "SaveManager.h"
#include "ComponentAssemblyDefiner.h"
#include "UIElementsGroup.h"
#include "FramerateUI.h"
#include "MainMenuUI.h"
#include "EditorCore/Inspector.h"


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

			GameObject* GetFocusedObject();
			bool SaveGameObject(GameObject& gameObject, std::string assetPath);
			bool LoadGameObjectIntoScene(std::string assetPath);
			bool LoadGameObjectAsPrefab(std::string assetPath);
			bool LoadScene(std::string assetPath);
			bool SaveScene(std::string assetPath);

		protected:
			void InitialiseAssets();
			void InitWorld();
			void InitialiseGame();
			void SelectObject(BoundsComponent* newSelection);
			bool TrySelectObject();

			void TestSaveGameObject(std::string assetPath);
			void TestLoadGameObject(std::string assetPath);
			void SaveWorld(std::string assetPath);
			void LoadWorld(std::string assetPath);
			void TestSave();
			void UpdateUI();
			void TestSaveByType();

			GameObject* CreateChildInstance(Vector3 offset, bool isStatic);
			GameObject* AddFloorToWorld(const Vector3& position);
			GameObject* AddSphereToWorld(const Vector3& position, float radius, float inverseMass = 10.0f, bool addToWorld = true);
			GameObject* AddRoleTToWorld(const Vector3& position, float inverseMass = 10.0f); // Anim
			GameObject* AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);
			GameObject* AddNavMeshToWorld(std::string navMeshFilePath, std::string meshId, const Vector3& position, Vector3 dimensions);
			GameObject* AddPlayerToWorld(const Vector3& position, NetworkSpawnData* spawnData = nullptr);
			void SaveUnityNavMeshPrefab(std::string assetPath, std::string navMeshObPath, std::string navMeshNavPath);

			void  CalculateCubeTransformations(const std::vector<Vector3>& vertices, Vector3& position, Vector3& scale, Quaternion& rotation);
			std::vector<Vector3>  GetVertices(Mesh* navigationMesh, int i);

			ComponentAssemblyDefiner* componentAssembly;
			std::string GetAssetPath(std::string pfabName);
#ifdef USEVULKAN
			GameTechVulkanRenderer*	renderer;
#else
			GameTechRendererInterface* renderer;
#endif
			PhysicsSystem* physics;
			GameWorld* world;
			Controller* controller;
			Inspector* inspectorBar;

			bool inSelectionMode;

			BoundsComponent* selectionObject = nullptr;
			NavigationPath outPath;
			NavigationMesh* navMesh = nullptr;

			BoundsComponent* lockedObject	= nullptr;
			Vector3 lockedOffset = Vector3(0, 14, 20);

			void LockCameraToObject(BoundsComponent* o) {
				lockedObject = o;
			}

			GameObject* objClosest = nullptr;
			UI::UISystem* uiSystem;
		};
	}
}

