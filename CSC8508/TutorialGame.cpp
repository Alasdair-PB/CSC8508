#include "TutorialGame.h"
#include "GameWorld.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "TextureLoader.h"

#include "GameObject.h"
#include "MaterialManager.h"
#include "Assets.h"
#include "PhysicsComponent.h"
#include "BoundsComponent.h"
#include "ItemComponent.h"

#ifdef USE_PS5
#include "../PS5Starter/GameTechAGCRenderer.h"
#include "../PS5Core/PS5Window.h"
#include "../PS5Core/PS5Controller.h"
#include "../UISystem/UIPlayStation.h"
#else
#include "GameTechRenderer.h"
#include "KeyboardMouseController.h"
#endif // USE_PS5


using namespace NCL;
using namespace CSC8508;

const static std::string folderPath = NCL::Assets::PFABDIR;

std::string TutorialGame::GetAssetPath(std::string pfabName) {
	return folderPath + pfabName;
}

GameObject* TutorialGame::LoadRoomPfab(std::string assetPath, Vector3 offset) {
	GameObject* myObjectToLoad = new GameObject(true);
	std::string pfabPath = GetAssetPath(assetPath);
	myObjectToLoad->Load(pfabPath);
	myObjectToLoad->GetTransform().SetPosition(offset);
	world->AddGameObject(myObjectToLoad);
	return myObjectToLoad;
}

void LoadControllerMappings(Controller* controller)
{
#ifdef USE_PS5
	controller->MapAxis(0, "Sidestep");
	controller->MapAxis(2, "Forward");
	controller->MapAxis(3, "XLook");
	controller->MapAxis(4, "YLook");
	controller->MapButton(KeyCodes::SHIFT, "Dash"); //Ps5 relevant buttons
	controller->MapButton(KeyCodes::SPACE, "Jump"); // Keep names
	controller->MapButton(KeyCodes::E, "Interact");
#else
	controller->MapAxis(0, "Sidestep");
	controller->MapAxis(2, "Forward");
	controller->MapAxis(3, "XLook");
	controller->MapAxis(4, "YLook");
	controller->MapButton(KeyCodes::SHIFT, "Dash");
	controller->MapButton(KeyCodes::SPACE, "Jump");
	controller->MapButton(KeyCodes::E, "Interact");
#endif
	controller->BindMappingsToHashIds();
}

void TutorialGame::InitialiseGame() {

	componentAssembly = new ComponentAssemblyDefiner();
	componentAssembly->InitializeMap();

	world->GetMainCamera().SetController(*controller);
	LoadControllerMappings(controller);
	InitialiseAssets();
	uiSystem = UI::UISystem::GetInstance();
	audioEngine = &AudioEngine::Instance();

	uiSystem->PushNewStack(framerate->frameUI, "Framerate");
	uiSystem->PushNewStack(audioSliders->audioSlidersUI, "Audio Sliders");

	//uiSystem->PushNewStack(mainMenuUI->menuUI, "Main Menu");
	//uiSystem->PushNewStack(inventoryUI->inventoryUI, "Inventory");

	/*uiSystem->PushNewStack(lobbySearchField->lobbySearchField, "Lobby Search Field");*/

	inSelectionMode = false;
	physics->UseGravity(true);
}

TutorialGame::TutorialGame()
{
	world = &GameWorld::Instance();
#ifdef USE_PS5
	NCL::PS5::PS5Window* w = (NCL::PS5::PS5Window*)Window::GetWindow();
	controller = w->GetController();
	renderer = new GameTechAGCRenderer(*world);
	UI::UIPlayStation::GetInstance()->SetPadHandle(static_cast<NCL::PS5::PS5Controller*>(controller)->GetHandle());
	UI::UIPlayStation::GetInstance()->InitMouse(static_cast<NCL::PS5::PS5Controller*>(controller)->GetUserId());
#else
	controller = new KeyboardMouseController(*Window::GetWindow()->GetKeyboard(), *Window::GetWindow()->GetMouse());
#ifdef USEVULKAN
	renderer = new GameTechVulkanRenderer(*world);
	renderer->Init();
	renderer->InitStructures();
#else
	renderer = new GameTechRenderer(*world);
#endif
#endif
	
	physics = new PhysicsSystem(*world);

	InitialiseGame();
}

void TutorialGame::InitialiseAssets() {
	MaterialManager::PushMesh("cube", renderer->LoadMesh("cube.msh"));
	MaterialManager::PushMesh("capsule", renderer->LoadMesh("capsule.msh"));
	MaterialManager::PushMesh("sphere", renderer->LoadMesh("sphere.msh"));
	MaterialManager::PushMesh("Role_T", renderer->LoadMesh("Role_T.msh"));
	MaterialManager::PushMesh("navMesh", renderer->LoadMesh("NavMeshObject.msh"));
	MaterialManager::PushMesh("player", renderer->LoadMesh("Astronaut.msh"));
	MaterialManager::PushTexture("basic", renderer->LoadTexture("checkerboard.png"));
	MaterialManager::PushTexture("player", renderer->LoadTexture("MiiCharacter.png"));
	MaterialManager::PushShader("basic", renderer->LoadShader("scene.vert", "scene.frag"));
	MaterialManager::PushShader("anim", renderer->LoadShader("skinning.vert", "scene.frag"));

	lockedObject = nullptr;
	InitWorld();
}

TutorialGame::~TutorialGame()	
{
}

void TutorialGame::UpdateGame(float dt)
{	
	world->UpdateWorld(dt);
	UpdateUI();
	mainMenu->Update(dt);
	renderer->Render();
	Debug::UpdateRenderables(dt);
	Window::GetWindow()->ShowOSPointer(true);
	physics->Update(dt);
	audioEngine->Update();
}

void TutorialGame::LoadWorld(std::string assetPath) {
	LoadDropZone(Vector3(85, 22, -60), Vector3(5,5,5));
	world->Load(assetPath);
}

void TutorialGame::InitWorld() 
{
	world->ClearAndErase();
	physics->Clear();

	//GameObject* room = LoadRoomPfab("room_A.pfab", Vector3(90, 90, -50));
	//GameObject* roomB = room->CopyGameObject();
	//room->SetEnabled(true);
	//roomB->GetTransform().SetPosition(Vector3(90, 60, -50));
	//roomB->SetEnabled(true);
	//world->AddGameObject(roomB);

	std::string assetPath = GetAssetPath("myScene.pfab"); 
	LoadWorld(assetPath);
}

void TutorialGame::UpdateUI() {
	uiSystem->StartFrame();
	framerateDelay += 1;

	if (framerateDelay > 10) {
		framerate->UpdateFramerate(Window::GetTimer().GetTimeDeltaSeconds());
		framerateDelay = 0;
	}

	if (mainMenuUI->GetMenuOption() != 0) {
		mainMenu->SetOption(mainMenuUI->GetMenuOption());
		uiSystem->RemoveStack("Main Menu");
		uiSystem->RemoveStack("Audio Sliders");
	}
	
	uiSystem->RenderFrame();
}


