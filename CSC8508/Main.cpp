#include "Debug.h"

#if EDITOR
#include "EditorGame.h"
#else 
#include "TutorialGame.h"
#include "NetworkedGame.h"
#endif

#include <chrono>
#include <thread>
#include <sstream>
#include <functional>
#include <cstdlib>

#include "Assets.h"
#include "Map/RoomPrefab.h"

#ifdef USE_PS5
#include "./PS5/GameTechAGCRenderer.h"
#include "PS5Window.h"
using namespace NCL;
using namespace PS5;

size_t sceUserMainThreadStackSize = 2 * 1024 * 1024;
extern const char sceUserMainThreadName[] = "TeamProjectGameMain";
int sceUserMainThreadPriority = SCE_KERNEL_PRIO_FIFO_DEFAULT;
size_t sceLibcHeapSize = 256 * 1024 * 1024;
#else
#include "Window.h"
#include "GameTechRenderer.h"
using namespace NCL;
using namespace CSC8508;
#endif

#if EDITOR
void UpdateWindow(Window* w, EditorGame* g)
#else 
void UpdateWindow(Window* w, NetworkedGame* g)
#endif
{
	float dt = w->GetTimer().GetTimeDeltaSeconds();
	w->SetTitle("Gametech frame time:" + std::to_string(std::roundf(1000.0f * dt)));
	g->UpdateGame(dt);
	Debug::UpdateRenderables(dt);
}

int main(int argc, char** argv) 
{

	//TutorialGame* tg = new TutorialGame();

	//std::string vectorIntPath = NCL::Assets::PFABDIR + "vector_data.pfab";
	//SaveManager::SaveGameData(vectorIntPath, SaveManager::CreateSaveDataAsset<std::vector<int>>(std::vector<int>{45}));
	//std::cout << SaveManager::LoadMyData<std::vector<int>>(vectorIntPath)[0] << std::endl;

	//GameObject* myObjectToLoad = new GameObject();
	//std::string pfabPath = NCL::Assets::PFABDIR + "room_A.pfab";
	//std::cout << "Two lines in!\n";
	//myObjectToLoad->Load(pfabPath);
	//std::cout << "Made it this far!\n";
	//myObjectToLoad->Save(NCL::Assets::PFABDIR + "room_A_TEST.pfab");
	//std::vector<Vector3> possibleItemSpawnLocations = { Vector3(0.0f, 0.0f, 20.0f) };
	//std::vector<DoorLocation> possibleDoorLocations = { DoorLocation(Vector3(100.0f, 0.0f, 0.0f), Vector3(1.0f, 0.0f, 0.0f)) };
	//myObjectToLoad->AddComponent<RoomPrefab>(possibleItemSpawnLocations, possibleDoorLocations);
	//myObjectToLoad->Save(NCL::Assets::PFABDIR + "room_A_TEST.pfab");

	//GameObject* room = new GameObject();
	// room->Load(NCL::Assets::PFABDIR + "room_A_TEST.pfab");
	//
	// RoomPrefab* prefab = room->TryGetComponent<RoomPrefab>();
	// std::cout << "Number of children: " << room->GetChildren().size() << '\n';
	// if (prefab) std::cout << "Found prefab component!\n";
	// else std::cout << "NOT found!\n";


	int wWidth	= 1920;
	int wHeight = 1200;
	if (argc == 3) {
		std::cout << "AHK Launch" << "\n";
		wWidth = std::atoi(argv[1]);
		wHeight = std::atoi(argv[2]);
	}
	else {
		std::cout << "Normal Launch" << "\n";
	}
	

#ifdef USE_PS5
	std::unique_ptr<PS5::PS5Window> w = std::make_unique<PS5::PS5Window>("PS5", 1920, 1080);

	NetworkedGame* g = new NetworkedGame();

	while (w->UpdateWindow()) {
		float dt = w->GetTimer().GetTimeDeltaSeconds();
		g->UpdateGame(dt);
		//Debug::UpdateRenderables(dt);
	}
#else 
	WindowInitialisation initInfo;
	initInfo.windowTitle = "CSC8508 Game technology!";
	initInfo.width = wWidth;
	initInfo.height = wHeight;

	Window* w = Window::CreateGameWindow(initInfo);

	if (!w->HasInitialised())
		return -1;
	w->ShowOSPointer(true);
	w->LockMouseToWindow(true);
	w->GetTimer().GetTimeDeltaSeconds();

#if EDITOR
	EditorGame* g = new EditorGame();
#else 
	NetworkedGame* g = new NetworkedGame();
#endif

	// GameObject* myObjectToLoad = new GameObject();
	// std::string pfabPath = NCL::Assets::PFABDIR + "room_A.pfab";
	// std::cout << "Two lines in!\n";
	// myObjectToLoad->Load(pfabPath);
	// std::cout << "Made it this far!\n";
	// //myObjectToLoad->Save(NCL::Assets::PFABDIR + "rooms/room_A_TEST.pfab");
	// std::vector<Vector3> possibleItemSpawnLocations = {
	// 	Vector3(0.0f, 5.0f, 25.0f),
	// 	Vector3(50.0f, 0.0f, 25.0f),
	// 	Vector3(0.0f, 0.0f, -25.0f),
	// 	Vector3(50.0f, 0.0f, -25.0f)
	// };
	// std::vector<DoorLocation> possibleDoorLocations = {
	// 	//DoorLocation(Vector3(-35.0f, 0.0f, 0.0f), Vector3(-1.0f, 0.0f, 0.0f)),
	// 	//DoorLocation(Vector3(25.0f, 0.0f, 60.0f), Vector3(0.0f, 0.0f, 1.0f)),
	// 	DoorLocation(Vector3(-60.0f, 0.0f, 0.0f), Vector3(-1.0f, 0.0f, 0.0f)),
	// 	DoorLocation(Vector3(0.0f, 0.0f, 60.0f), Vector3(0.0f, 0.0f, 1.0f)),
	// };
	// myObjectToLoad->AddComponent<RoomPrefab>(possibleItemSpawnLocations, possibleDoorLocations);
	// myObjectToLoad->Save(NCL::Assets::PFABDIR + "rooms/RP_A.pfab");

	// GameObject* room = new GameObject();
	// room->Load(NCL::Assets::PFABDIR + "rooms/RP_A.pfab");
	// RoomPrefab* prefab = room->TryGetComponent<RoomPrefab>();
	// if (prefab) {
	// 	std::cout << "Prefab found!\n";
	// 	for (Vector3 loc : prefab->GetItemSpawnLocations()) {
	// 		std::cout << "Item spawn location: " << loc.x << ", " << loc.y << ", " << loc.z << '\n';
	// 	}
	// 	for (DoorLocation dl : prefab->GetDoorLocations()) {
	// 		std::cout << "Door location: " << dl.pos.x << ", " << dl.pos.y << ", " << dl.pos.z << '\n';
	// 		std::cout << "Door direction: " << dl.dir.x << ", " << dl.dir.y << ", " << dl.dir.z << '\n';
	// 	}
	// }
	// else std::cout << "Prefab NOT found! :(\n";


	//return 0;

	while (w->UpdateWindow() && !Window::GetKeyboard()->KeyDown(KeyCodes::ESCAPE))
	{
		float dt = w->GetTimer().GetTimeDeltaSeconds();
		g->UpdateGame(dt);
		Debug::UpdateRenderables(dt);
	}
	Window::DestroyGameWindow();

	#if EDITOR
		g->~EditorGame();
	#else 
		g->~NetworkedGame();
	#endif
#endif
	
}