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


using std::vector;

namespace NCL {
	namespace CSC8508 {

		struct NetworkSpawnData
		{
			int objId;
			int ownId;
			bool clientOwned;
		};

		class TutorialGame {
		public:
			TutorialGame();
			~TutorialGame();
			virtual void UpdateGame(float dt);

		protected:
			void InitialiseAssets();
			void InitWorld();
			void InitialiseGame();

			void UpdateObjectSelectMode(float dt);
			bool SelectObject();


			void TestSaveGameObject(std::string assetPath);
			void TestLoadGameObject(std::string assetPath);
			void SaveWorld(std::string assetPath);
			void LoadWorld(std::string assetPath);
			void TestSave();
			void UpdateUI();

			GameObject* CreateChildInstance(Vector3 offset, bool isStatic);
			GameObject* AddFloorToWorld(const Vector3& position);
			GameObject* AddSphereToWorld(const Vector3& position, float radius, float inverseMass = 10.0f, bool addToWorld = true);
			GameObject* AddRoleTToWorld(const Vector3& position, float inverseMass = 10.0f); // Anim
			GameObject* AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);
			GameObject* AddNavMeshToWorld(const Vector3& position, Vector3 dimensions);
			GameObject* AddPlayerToWorld(const Vector3& position, NetworkSpawnData* spawnData = nullptr);

			void  CalculateCubeTransformations(const std::vector<Vector3>& vertices, Vector3& position, Vector3& scale, Quaternion& rotation);
			std::vector<Vector3>  GetVertices(Mesh* navigationMesh, int i);

			MainMenu* GetMainMenu() { return mainMenu; }
			ComponentAssemblyDefiner* componentAssembly;

#ifdef USEVULKAN
			GameTechVulkanRenderer*	renderer;
#else
			GameTechRendererInterface* renderer;
#endif
			PhysicsSystem* physics;
			GameWorld* world;
			Controller* controller;

			bool inSelectionMode;

			BoundsComponent* selectionObject = nullptr;
			NavigationPath outPath;
			NavigationMesh* navMesh = nullptr;

			MainMenu* mainMenu = nullptr;
			BoundsComponent* lockedObject	= nullptr;
			Vector3 lockedOffset = Vector3(0, 14, 20);

			void LockCameraToObject(BoundsComponent* o) {
				lockedObject = o;
			}

			GameObject* objClosest = nullptr;
			UI::UISystem* uiSystem;

			UI::FramerateUI* framerate = new UI::FramerateUI;
			UI::MainMenuUI* mainMenuUI = new UI::MainMenuUI;
			UI::AudioSliders* audioSliders = new UI::AudioSliders;
			UI::Healthbar* healthbar = new UI::Healthbar;
			UI::LobbySearch* lobbySearchField = new UI::LobbySearch;

			float framerateDelay = 0;
		};
	}
}

