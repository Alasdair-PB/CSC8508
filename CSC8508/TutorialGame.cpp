#include "TutorialGame.h"
#include "GameWorld.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "TextureLoader.h"

#include "GameObject.h"
#include "TransformNetworkComponent.h"

#include "PositionConstraint.h"
#include "MeshManager.h"
#include "MaterialManager.h"

#include "OrientationConstraint.h"

using namespace NCL;
using namespace CSC8508;

struct MyX {
	MyX() : x(0) {}
	MyX(int x) :x(x) {}
	int x;
};

const static std::string folderPath = "../../Assets/Pfabs/"; 

std::string GetAssetPath(std::string pfabName) {
	return folderPath + pfabName;
}


void TestSaveByType() {
	std::string vectorIntPath = GetAssetPath("vector_data.pfab");
	std::string intPath = GetAssetPath("int_data.pfab");
	std::string structPath = GetAssetPath("struct_data.pfab");

	SaveManager::SaveGameData(vectorIntPath, SaveManager::CreateSaveDataAsset<std::vector<int>>(std::vector<int>{45}));
	std::cout << SaveManager::LoadMyData<std::vector<int>>(vectorIntPath)[0] << std::endl;
	SaveManager::SaveGameData(intPath, SaveManager::CreateSaveDataAsset<int>(45));
	std::cout << SaveManager::LoadMyData<int>(intPath) << std::endl;
	SaveManager::SaveGameData(structPath, SaveManager::CreateSaveDataAsset<MyX>(MyX(2)));
	std::cout << SaveManager::LoadMyData<MyX>(structPath).x << std::endl;
}

void TestSaveGameObject() {
	std::string gameObjectPath = GetAssetPath("object_data.pfab");
	GameObject* myObjectToSave = new GameObject();
	PhysicsComponent* phys = myObjectToSave->AddComponent<PhysicsComponent>();

	myObjectToSave->Save(gameObjectPath);
	myObjectToSave->Load(gameObjectPath);
}

void TestSave() {
	TestSaveByType();
	TestSaveGameObject();
}

void DreamFrameWork() {
	// Create new game World
	// Load Controller Map from save data
	// Load Camera and init from save data
	// Load Physics Settings from save data
	// Load scene from save data
	// Set UI either from components or by direct call
}

TutorialGame::TutorialGame() : controller(*Window::GetWindow()->GetKeyboard(), *Window::GetWindow()->GetMouse()) 
{
	world = new GameWorld();
#ifdef USEVULKAN
	renderer	= new GameTechVulkanRenderer(*world);
	renderer->Init();
	renderer->InitStructures();
#else 
	renderer = new GameTechRenderer(*world);
#endif
	physics = new PhysicsSystem(*world);

	inSelectionMode = false;
	world->GetMainCamera().SetController(controller);

	controller.MapAxis(0, "Sidestep");
	controller.MapAxis(2, "Forward");
	controller.MapAxis(3, "XLook");
	controller.MapAxis(4, "YLook");

	InitialiseAssets();	
	
	physics->UseGravity(true);
	uiSystem = new UISystem(Window::GetHandle());
	renderer->SetUISystem(uiSystem);

	TestSave();
}

void TutorialGame::InitialiseAssets() {

	MeshManager::PushMesh("cube", renderer->LoadMesh("cube.msh"));
	MeshManager::PushMesh("capsule", renderer->LoadMesh("capsule.msh"));
	MeshManager::PushMesh("sphere", renderer->LoadMesh("sphere.msh"));
	MeshManager::PushMesh("navMesh", renderer->LoadMesh("NavMeshObject.msh"));
	MaterialManager::PushTexture("basic", renderer->LoadTexture("checkerboard.png"));
	MaterialManager::PushShader("basic", renderer->LoadShader("scene.vert", "scene.frag"));

	lockedObject = nullptr;
	InitWorld();
}

TutorialGame::~TutorialGame()	
{
	MeshManager::CleanUp();
	MaterialManager::CleanUp();
	ComponentManager::CleanUp();

	delete physics;
	delete renderer;
	delete world;
	delete navMesh;
	delete uiSystem;
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
	DrawUIElements();
	mainMenu->Update(dt);
	renderer->Render();	
	Debug::UpdateRenderables(dt);

	world->UpdateWorld(dt);
	Window::GetWindow()->ShowOSPointer(true);
	physics->Update(dt);
}

void TutorialGame::InitWorld() 
{
	world->ClearAndErase();
	physics->Clear();
	AddNavMeshToWorld(Vector3(0, 0, 0), Vector3(1, 1, 1));

	std::string assetPath = GetAssetPath("myScene.pfab");
	world->Save(assetPath);
	world->Load(assetPath);
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

void TutorialGame::DrawUIElements() {
	framerateDelay += 1;

	uiSystem->StartFrame();
	if (framerateDelay > 10) {
		latestFramerate = Window::GetTimer().GetTimeDeltaSeconds();
		framerateDelay = 0;
	}
	uiSystem->DisplayFramerate(latestFramerate);
}
