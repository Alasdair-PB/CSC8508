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
	GameObject* myObjectToLoad = new GameObject();
	std::string pfabPath = GetAssetPath(assetPath);
	myObjectToLoad->Load(pfabPath);
	myObjectToLoad->GetTransform().SetPosition(myObjectToLoad->GetTransform().GetPosition() + offset);
	world->AddGameObject(myObjectToLoad);
	return myObjectToLoad;
}

void LoadControllerMappings(Controller* controller)
{
	controller->MapAxis(0, "Sidestep");
	controller->MapAxis(2, "Forward");
	controller->MapAxis(3, "XLook");
	controller->MapAxis(4, "YLook");
	controller->MapButton(KeyCodes::SHIFT, "Dash");
	controller->MapButton(KeyCodes::SPACE, "Jump");
	controller->MapButton(KeyCodes::E, "Interact");
}

void TutorialGame::InitialiseGame() {

	componentAssembly = new ComponentAssemblyDefiner();
	componentAssembly->InitializeMap();

	world->GetMainCamera().SetController(*controller);
	LoadControllerMappings(controller);

	std::string vectorIntPath = GetAssetPath("vector_data.pfab");
	SaveManager::SaveGameData(vectorIntPath, SaveManager::CreateSaveDataAsset<std::vector<int>>(std::vector<int>{45}));
	std::cout << SaveManager::LoadMyData<std::vector<int>>(vectorIntPath)[0] << std::endl;

	InitialiseAssets();
	uiSystem = UI::UISystem::GetInstance();

	uiSystem->PushNewStack(framerate->frameUI, "Framerate");
	uiSystem->PushNewStack(mainMenuUI->menuUI, "Main Menu");
	uiSystem->PushNewStack(audioSliders->audioSlidersUI, "Audio Sliders");
	uiSystem->PushNewStack(inventoryUI->inventoryUI, "Inventory");
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
	MaterialManager::CleanUp();
	ComponentManager::CleanUp();

	delete physics;
	delete renderer;
	delete world;
	delete controller;
	delete navMesh;

	delete framerate;
	delete mainMenuUI;
	delete audioSliders;
	delete healthbar;
	delete lobbySearchField;
	delete inventoryUI;
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
}

void TutorialGame::LoadWorld(std::string assetPath) {
	world->Load(assetPath);
}

void TutorialGame::InitWorld() 
{
	world->ClearAndErase();
	physics->Clear();
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
		uiSystem->RemoveStack("Inventory");
		uiSystem->PushNewStack(healthbar->healthbar, "Healthbar");
	}

	uiSystem->RenderFrame();
}


