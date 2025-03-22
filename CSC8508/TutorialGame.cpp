#include "TutorialGame.h"
#include "GameWorld.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "TextureLoader.h"

#include "GameObject.h"
#include "TransformNetworkComponent.h"

#include "PositionConstraint.h"
#include "MaterialManager.h"
#include "OrientationConstraint.h"
#include "Assets.h"
#include "PhysicsComponent.h"
#include "BoundsComponent.h"


#ifdef USE_PS5
#include "../PS5Starter/GameTechAGCRenderer.h"
#include "../PS5Core/PS5Window.h"
#include "../PS5Core/PS5Controller.h"
#else
#include "GameTechRenderer.h"
#include "KeyboardMouseController.h"
#endif // USE_PS5


using namespace NCL;
using namespace CSC8508;

struct MyX {
	MyX() : x(0) {}
	MyX(int x) :x(x) {}
	int x;
};

enum testGuy {X};

const static std::string folderPath = NCL::Assets::PFABDIR;

std::string GetAssetPath(std::string pfabName) {
	return folderPath + pfabName;
}

void TestSaveByType() {
	std::string vectorIntPath = GetAssetPath("vector_data.pfab");
	std::string intPath = GetAssetPath("int_data.pfab");
	std::string structPath = GetAssetPath("struct_data.pfab");
	std::string enumPath = GetAssetPath("enum_data.pfab");

	SaveManager::SaveGameData(vectorIntPath, SaveManager::CreateSaveDataAsset<std::vector<int>>(std::vector<int>{45}));
	std::cout << SaveManager::LoadMyData<std::vector<int>>(vectorIntPath)[0] << std::endl;
	SaveManager::SaveGameData(intPath, SaveManager::CreateSaveDataAsset<int>(45));
	std::cout << SaveManager::LoadMyData<int>(intPath) << std::endl;
	SaveManager::SaveGameData(enumPath, SaveManager::CreateSaveDataAsset<MyX>(MyX(2)));
	std::cout << SaveManager::LoadMyData<MyX>(enumPath).x << std::endl;
	SaveManager::SaveGameData(structPath, SaveManager::CreateSaveDataAsset<testGuy>(X));
	std::cout << SaveManager::LoadMyData<testGuy>(structPath) << std::endl;
}

GameObject* TutorialGame::CreateChildInstance(Vector3 offset, bool isStatic) {
	GameObject* myObjectToSave = AddSphereToWorld(offset, 1, isStatic ? 0 : 10, false);
	return myObjectToSave;
}

void TutorialGame::TestSaveGameObject(std::string assetPath) {

	Vector3 position = Vector3(90 + 10, 22, -50);
	GameObject* myObjectToSaveA = AddSphereToWorld(position, 1, 10.0f, false);
	GameObject* child = CreateChildInstance(Vector3(5, 0, 0), false);
	child->AddChild(CreateChildInstance(Vector3(5, 0, 0), true));

	myObjectToSaveA->AddChild(child);
	myObjectToSaveA->Save(assetPath);
	world->AddGameObject(myObjectToSaveA);	
}

void TutorialGame::TestLoadGameObject(std::string assetPath) {
	GameObject* myObjectToLoad = new GameObject();
	myObjectToLoad->Load(assetPath);
	myObjectToLoad->GetTransform().SetPosition(myObjectToLoad->GetTransform().GetPosition() + Vector3(-2, 0, 2));
	//myObjectToLoad->SetEnabled(false);
	world->AddGameObject(myObjectToLoad);
}

void TutorialGame::TestSave() {
	std::string gameObjectPath = GetAssetPath("object_data.pfab");
	TestSaveByType();
	TestSaveGameObject(gameObjectPath);
	//TestLoadGameObject(gameObjectPath);
}

void LoadControllerMappings(Controller* controller)
{
	controller->MapAxis(0, "Sidestep");
	controller->MapAxis(2, "Forward");
	controller->MapAxis(3, "XLook");
	controller->MapAxis(4, "YLook");
	controller->MapButton(KeyCodes::SHIFT, "Dash");
	controller->MapButton(KeyCodes::SPACE, "Jump");

}

void TutorialGame::InitialiseGame() {

	componentAssembly = new ComponentAssemblyDefiner();
	componentAssembly->InitializeMap();

	world->GetMainCamera().SetController(*controller);
	LoadControllerMappings(controller);

	InitialiseAssets();
	uiSystem = UI::UISystem::GetInstance();

	uiSystem->PushNewStack(framerate->frameUI, "Framerate");
	uiSystem->PushNewStack(mainMenuUI->menuUI, "Main Menu");
	uiSystem->PushNewStack(audioSliders->audioSlidersUI, "Audio Sliders");
	uiSystem->PushNewStack(lobbySearchField->lobbySearchField, "Lobby Search Field");

	inSelectionMode = false;
	physics->UseGravity(true);
}

TutorialGame::TutorialGame()
{
	world = new GameWorld();
#ifdef USE_PS5
	NCL::PS5::PS5Window* w = (NCL::PS5::PS5Window*)Window::GetWindow();
	controller = w->GetController();
	renderer = new GameTechAGCRenderer(*world);
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
	MaterialManager::PushMesh("Role_T", renderer->LoadMesh("Role_T.msh"));
	MaterialManager::PushTexture("basic", renderer->LoadTexture("checkerboard.png"));
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
}

void TutorialGame::UpdateObjectSelectMode(float dt) {

	RayCollision closestCollision;
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::K) && selectionObject) {
		Vector3 rayPos;
		Vector3 rayDir;

		rayDir = selectionObject->GetGameObject().GetTransform().GetOrientation() * Vector3(0, 0, -1);
		rayPos = selectionObject->GetGameObject().GetTransform().GetPosition();

		Ray r = Ray(rayPos, rayDir);
		bool hit = world->Raycast(r, closestCollision, true, selectionObject, new std::vector<Layers::LayerID>({ Layers::LayerID::Player,  Layers::LayerID::Enemy }));

		if (hit)
		{
			if (objClosest)
				objClosest->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
			objClosest = (GameObject*)closestCollision.node;
			objClosest->GetRenderObject()->SetColour(Vector4(1, 0, 1, 1));
		}
	}
	SelectObject();
}

void TutorialGame::UpdateGame(float dt)
{
	UpdateUI();
	mainMenu->Update(dt);
	renderer->Render();
	Debug::UpdateRenderables(dt);

	world->UpdateWorld(dt);
	Window::GetWindow()->ShowOSPointer(true);
	physics->Update(dt);
}

void TutorialGame::LoadWorld(std::string assetPath) {
	world->Load(assetPath);
}

void TutorialGame::SaveWorld(std::string assetPath) {
	auto x = AddNavMeshToWorld(Vector3(0, 0, 0), Vector3(1, 1, 1));
	delete x;
	world->Save(assetPath);
}

const bool load = true;

void TutorialGame::InitWorld() 
{
	world->ClearAndErase();
	physics->Clear();
	//TestSave();
	std::string assetPath = GetAssetPath("myScene.pfab"); 
	//load ? LoadWorld(assetPath) : SaveWorld(assetPath);
	LoadWorld(assetPath);
	AddRoleTToWorld(Vector3(90, 30, -52)); //PS5
}

bool TutorialGame::SelectObject() {
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::Q))
		inSelectionMode = !inSelectionMode;
	if (inSelectionMode) {
		if (Window::GetMouse()->ButtonDown(NCL::MouseButtons::Left)) {

			RenderObject* ro = selectionObject->GetGameObject().GetRenderObject();
			if (selectionObject)
			{
				ro->SetColour(Vector4(1, 1, 1, 1));
				selectionObject = nullptr;
			}

			Ray ray = CollisionDetection::BuildRayFromMouse(world->GetMainCamera());
			RayCollision closestCollision;
			if (world->Raycast(ray, closestCollision, true))
			{
				selectionObject = (BoundsComponent*)closestCollision.node;
				ro->SetColour(Vector4(0, 1, 0, 1));
				return true;
			}
			else
				return false;
		}
	}
	return false;
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
		uiSystem->RemoveStack("Lobby Search Field");
		uiSystem->PushNewStack(healthbar->healthbar, "Healthbar");
	}

	uiSystem->RenderFrame();
}


