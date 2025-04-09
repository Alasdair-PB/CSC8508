#pragma once
#include "Controller.h"
#include "NavigationGrid.h"
#include "NavigationMesh.h"
#include "MainMenu.h"
#include "Math.h"
#include "IGameTechRenderer.h"
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
#include "../DungeonGeneration/DoorLocation.h"

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
//#include "
// UI.h"

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

		class GameCore {
		public:
			GameCore();
			~GameCore();
			virtual void UpdateGame(float dt);
		protected:
			void InitialiseAssets();
			void InitWorld();
			void InitialiseGame();

			void LoadWorld(std::string assetPath);
			void UpdateUI();
			void RefreshSpawnLocals();

			std::string GetAssetPath(std::string pfabName);
			void LoadDungeon(Vector3 offset);
			GameObject* LoadRoomPfab(std::string assetPath, Vector3 offset);
			GameObject* AddPlayerToWorld(const Vector3& position, NetworkSpawnData* spawnData = nullptr);
			GameObject* Loaditem(const Vector3& position, NetworkSpawnData* spawnData = nullptr);
			GameObject* LoadGameManager(const Vector3& position, NetworkSpawnData* spawnData = nullptr);
			GameObject* LoadDropZone(const Vector3& position, Vector3 dimensions, Tag tag);
			GameObject* AddDungeonToWorld(Transform const& transform, DoorLocation const& entryPosition, int roomCount);

			MainMenu* GetMainMenu() { return mainMenu; }
			ComponentAssemblyDefiner* componentAssembly;

			Vector3 GetSpawnLocation(int item);

#ifdef USEVULKAN
			GameTechVulkanRenderer*	renderer;
#else
			IGameTechRenderer* renderer;
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

			vector<Vector3> locals = vector<Vector3>();
			UI::FramerateUI* framerate = new UI::FramerateUI;
			UI::MainMenuUI* mainMenuUI = new UI::MainMenuUI;
			UI::LobbySearch* lobbySearchField = new UI::LobbySearch;
			UI::InventoryUI* inventoryUI = new UI::InventoryUI;
			UI::AudioSliders* audioSliders = new UI::AudioSliders;
			/*UI::PauseUI* pauseUI = new UI::PauseUI;*/

			float framerateDelay = 0;
			int seed;
			int itemCount; 

#if EOSBUILD
			UI::EOSMenuUI* eosMenuUI = new UI::EOSMenuUI;
			UI::EOSLobbyMenuUI* eosLobbyMenuUI = new UI::EOSLobbyMenuUI(false, "", "", 0);

			bool eosLobbyMenuCreated = false;
#endif
		};
	}
}

