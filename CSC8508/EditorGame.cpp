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

void EditorGame::DeleteSelectionObject() {
	selectionObject = nullptr;
	physics->Clear();
}

void EditorGame::InitWorld()
{
	world->ClearAndErase();
	physics->Clear();
	//LoadWorld(GetAssetPath("myScene.pfab"));
}

void EditorGame::SelectObject(BoundsComponent* newSelection) {
	if (!newSelection || newSelection == selectionObject) return; 
	selectionObject = newSelection;
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


