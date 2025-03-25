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

			void TestLoadGameObject(std::string assetPath);
			GameObject* LoadRoomPfab(std::string assetPath, Vector3 offset);
			void LoadWorld(std::string assetPath);
			void UpdateUI();

			GameObject* AddRoleTToWorld(const Vector3& position, float inverseMass = 10.0f); // Anim
			GameObject* AddPlayerToWorld(const Vector3& position, NetworkSpawnData* spawnData = nullptr);

			MainMenu* GetMainMenu() { return mainMenu; }
			ComponentAssemblyDefiner* componentAssembly;

#ifdef USEVULKAN
			GameTechVulkanRenderer*	renderer;
#else
			GameTechRendererInterface* renderer;
#endif
			PhysicsSystem* physics;
			AudioEngine* audioEngine = nullptr;
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
			UI::InventoryUI* inventoryUI = new UI::InventoryUI;

			float framerateDelay = 0;
		};
	}
}

