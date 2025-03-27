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

#if EOSBUILD
#include "MainMenuUI.h"
#include "EOSMenuUI.h"
#include "LobbySearch.h"
#include "EOSLobbyMenuUI.h"
#endif

#include "AudioSliders.h"
#include "FramerateUI.h"
#include "MainMenuUI.h"
#include "StaminaBar.h"
#include "LobbySearch.h"

using std::vector;

namespace NCL {
	namespace CSC8508 {

		struct NetworkSpawnData
		{
			int objId;
			int ownId;
			size_t pfab;
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

			void LoadWorld(std::string assetPath);
			void UpdateUI();
			std::string GetAssetPath(std::string pfabName);

			GameObject* LoadRoomPfab(std::string assetPath, Vector3 offset);
			GameObject* AddPlayerToWorld(const Vector3& position, NetworkSpawnData* spawnData = nullptr);
			GameObject* Loaditem(const Vector3& position, NetworkSpawnData* spawnData = nullptr);
			GameObject* LoadDropZone(const Vector3& position, Vector3 dimensions);
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
			UI::LobbySearch* lobbySearchField = new UI::LobbySearch;
			UI::InventoryUI* inventoryUI = new UI::InventoryUI;
			UI::AudioSliders* audioSliders = new UI::AudioSliders;

			float framerateDelay = 0;

#if EOSBUILD
			UI::EOSMenuUI* eosMenuUI = new UI::EOSMenuUI;
			UI::EOSLobbyMenuUI* eosLobbyMenuUI = new UI::EOSLobbyMenuUI(false, "", "", 0);

			bool eosLobbyMenuCreated = false;
#endif
		};
	}
}

