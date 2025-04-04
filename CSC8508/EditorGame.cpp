#include "EditorGame.h"
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
#include "EditorCore/Hierarchy.h"
#include "EditorCore/Inspector.h"
#include "EditorCore/ToolsBar.h"

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

std::string EditorGame::GetAssetPath(std::string pfabName) {
	return folderPath + pfabName;
}

void EditorGame::TestSaveByType() {
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

GameObject* EditorGame::CreateChildInstance(Vector3 offset, bool isStatic) {
	GameObject* myObjectToSave = AddSphereToWorld(offset, 1, isStatic ? 0 : 10);
	return myObjectToSave;
}

void EditorGame::TestSaveGameObject(std::string assetPath) {

	Vector3 position = Vector3(90 + 10, 22, -50);
	GameObject* myObjectToSaveA = AddSphereToWorld(position, 1, 10.0f);
	GameObject* child = CreateChildInstance(Vector3(5, 0, 0), false);
	child->AddChild(CreateChildInstance(Vector3(5, 0, 0), true));

	myObjectToSaveA->AddChild(child);
	myObjectToSaveA->Save(assetPath);
	world->AddGameObject(myObjectToSaveA);	
}

void EditorGame::TestLoadGameObject(std::string assetPath) {
	GameObject* myObjectToLoad = new GameObject();
	myObjectToLoad->Load(assetPath);
	myObjectToLoad->GetTransform().SetPosition(myObjectToLoad->GetTransform().GetPosition() + Vector3(2, 0, 2));
	world->AddGameObject(myObjectToLoad);
}

void EditorGame::TestSave() {
	std::string gameObjectPath = GetAssetPath("object_data.pfab");
	TestSaveByType();
	TestSaveGameObject(gameObjectPath);
	TestLoadGameObject(gameObjectPath);
}


ComponentAssemblyDefiner* EditorGame::GetDefiner() {
	return componentAssembly;
}

void LoadControllerMappings(Controller* controller)
{
	controller->MapAxis(0, "Sidestep");
	controller->MapAxis(1, "Up");
	controller->MapAxis(2, "Forward");
	controller->MapAxis(3, "XLook");
	controller->MapAxis(4, "YLook");
	controller->MapButton(KeyCodes::SHIFT, "Dash");
	controller->MapButton(KeyCodes::SPACE, "Jump");

}

void EditorGame::InitialiseGame() {

	componentAssembly = new ComponentAssemblyDefiner();
	componentAssembly->InitializeMap();

	world->GetMainCamera().SetController(*controller);
	LoadControllerMappings(controller);

	editorCamera = new EditorCamera(&world->GetMainCamera(), controller);

	InitialiseAssets();
	uiSystem = UI::UISystem::GetInstance();
	windowManager.AddWindow(new Inspector());
	windowManager.AddWindow(new Hierarchy());
	windowManager.AddWindow(new ToolsBar());

	inSelectionMode = false;
	physics->UseGravity(true);
}

EditorGame::EditorGame() : windowManager(EditorWindowManager::Instance())
{
	instance = this;
	world = &GameWorld::Instance(); 
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

void EditorGame::InitialiseAssets() {
	MaterialManager::PushMesh("cube", renderer->LoadMesh("cube.msh"));
	MaterialManager::PushMesh("capsule", renderer->LoadMesh("capsule.msh"));
	MaterialManager::PushMesh("sphere", renderer->LoadMesh("sphere.msh"));
	MaterialManager::PushMesh("navMesh", renderer->LoadMesh("NavMeshObject.msh"));
	MaterialManager::PushMesh("Role_T", renderer->LoadMesh("Role_T.msh"));
	MaterialManager::PushTexture("basic", renderer->LoadTexture("checkerboard.png"));
	MaterialManager::PushShader("basic", renderer->LoadShader("scene.vert", "scene.frag"));
	MaterialManager::PushShader("anim", renderer->LoadShader("skinning.vert", "scene.frag"));
	InitWorld();
}

EditorGame::~EditorGame()
{
	uiSystem->RemoveStack("InpsectorBar");
	uiSystem->RemoveStack("InpsectorBar");

	//MaterialManager::CleanUp();
	ComponentManager::CleanUp();

	delete physics;
	delete controller;
	delete navMesh;
}

void EditorGame::UpdateGame(float dt)
{	
	TrySelectObject();
	editorCamera->Update(dt);
	UpdateUI();
	renderer->Render();
	windowManager.RenderFocus();
	Debug::UpdateRenderables(dt);
	world->UpdateWorld(dt);
	Window::GetWindow()->ShowOSPointer(true);
}

void EditorGame::LoadWorld(std::string assetPath) {
	world->Load(assetPath);
}

void EditorGame::SaveWorld(std::string assetPath) {
	auto x = AddNavMeshToWorld("NavMeshObject.msh", "smalltest.navmesh", Vector3(0, 0, 0), Vector3(1, 1, 1));
	delete x;
	world->Save(assetPath);
}

void EditorGame::DeleteSelectionObject() {
	selectionObject = nullptr;
	physics->Clear();
}

//const bool load = true;

void EditorGame::InitWorld()
{
	world->ClearAndErase();
	physics->Clear();
	//std::string assetPath = GetAssetPath("myScene.pfab"); 
	//load ? LoadWorld(assetPath) : SaveWorld(assetPath);
}

void EditorGame::SelectObject(BoundsComponent* newSelection) {
	if (!newSelection || newSelection == selectionObject) return; 
	RenderObject* ro;
	if (selectionObject) {
		ro = selectionObject->GetGameObject().GetRenderObject();
		ro->SetColour(Vector4(1, 1, 1, 1));
		windowManager.EndFocus();
	}
	selectionObject = newSelection;
	ro = selectionObject->GetGameObject().GetRenderObject();
	ro->SetColour(Vector4(0, 1, 0, 1));
	windowManager.SetFocus(&(selectionObject->GetGameObject()));
}

bool EditorGame::TrySelectObject() {
	if (Window::GetMouse()->ButtonDown(NCL::MouseButtons::Left) && !ImGui::GetIO().WantCaptureMouse) {

		Ray ray = CollisionDetection::BuildRayFromMouse(world->GetMainCamera());
		RayCollision closestCollision;
		if (world->Raycast(ray, closestCollision, true))
		{
			SelectObject((BoundsComponent*)closestCollision.node);
			return true;
		}
		else
			return false;
	}
	return false;
}

void EditorGame::UpdateUI() {
	uiSystem->StartFrame();
	uiSystem->RenderFrame();
	uiSystem->EndFrame();
}


