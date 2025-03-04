#include "TutorialGame.h"
#include "GameWorld.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "TextureLoader.h"

#include "GameObject.h"
#include "TransformNetworkComponent.h"

#include "PositionConstraint.h"
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

	//SaveManager::SaveGameData(vectorIntPath, SaveManager::CreateSaveDataAsset<std::vector<int>>(std::vector<int>{45}));
	std::cout << SaveManager::LoadMyData<std::vector<int>>(vectorIntPath)[0] << std::endl;
	//SaveManager::SaveGameData(intPath, SaveManager::CreateSaveDataAsset<int>(45));
	std::cout << SaveManager::LoadMyData<int>(intPath) << std::endl;
	//SaveManager::SaveGameData(structPath, SaveManager::CreateSaveDataAsset<MyX>(MyX(2)));
	std::cout << SaveManager::LoadMyData<MyX>(structPath).x << std::endl;
}

void TestSaveGameObject() {
	std::string gameObjectPath = GetAssetPath("object_data.pfab");
	GameObject* myObjectToSave = new GameObject();
	PhysicsComponent* phys = myObjectToSave->AddComponent<PhysicsComponent>();

	//myObjectToSave->Save(gameObjectPath);
	myObjectToSave->Load(gameObjectPath);
}

void TestSave() {
	TestSaveByType();
	TestSaveGameObject();
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

	forceMagnitude	= 10.0f;
	useGravity		= false;
	inSelectionMode = false;

	world->GetMainCamera().SetController(controller);

	controller.MapAxis(0, "Sidestep");
	controller.MapAxis(2, "Forward");
	controller.MapAxis(3, "XLook");
	controller.MapAxis(4, "YLook");

	InitialiseAssets();	
	
	physics->UseGravity(true);
	world->UpdateWorld(0.1f);
	physics->Update(0.1f);

	uiSystem = new UISystem(Window::GetHandle());
	renderer->SetUISystem(uiSystem);

	TestSave();
}


void TutorialGame::SetPause(bool state) {
	inPause = state;
}

void TutorialGame::EndGame(bool hasWon) {
	inPause = true;
}

void TutorialGame::InitialiseAssets() {
	cubeMesh	= renderer->LoadMesh("cube.msh");
	navigationMesh = renderer->LoadMesh("NavMeshObject.msh");
	capsuleMesh = renderer->LoadMesh("capsule.msh");
	sphereMesh = renderer->LoadMesh("sphere.msh");


	basicTex	= renderer->LoadTexture("checkerboard.png");
	basicShader = renderer->LoadShader("scene.vert", "scene.frag");

	InitCamera();
	InitWorld();
}

TutorialGame::~TutorialGame()	
{
	delete cubeMesh;	
	delete capsuleMesh;
	delete sphereMesh;


	delete basicTex;
	delete basicShader;

	delete physics;
	delete renderer;
	delete world;

	delete navigationMesh;
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
			if (objClosest) {
				objClosest->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
			}
			objClosest = (GameObject*)closestCollision.node;
			objClosest->GetRenderObject()->SetColour(Vector4(1, 0, 1, 1));
		}
	}

	SelectObject();
	MoveSelectedObject();
}

bool TutorialGame::OnEndGame(float dt) {
	/*if (endGame) {
		renderer->Render();
		renderer->Update(dt);
		Debug::UpdateRenderables(dt);
		return true;
	}*/

	return false;
}

void TutorialGame::UpdateScore(float score) {
	this->score += score;
}

void TutorialGame::UpdateDrawScreen(float dt) {
	time += dt;
	Debug::Print("Score: " + std::to_string(score), Vector2(70, 20));
	Debug::Print("Time: " + std::to_string(time), Vector2(70, 10));
}

void TutorialGame::UpdateGame(float dt) 
{
	if (OnEndGame(dt))
		return; 

	DrawUIElements();
	mainMenu->Update(dt);
	renderer->Render();	
	Debug::UpdateRenderables(dt);

	if (inPause)
		return;

	UpdateDrawScreen(dt);
	world->UpdateWorld(dt);

	Window::GetWindow()->ShowOSPointer(true);
	//Window::GetWindow()->LockMouseToWindow(true);
	physics->Update(dt);
}

void TutorialGame::LockedObjectMovement() 
{
	Matrix4 view = world->GetMainCamera().BuildViewMatrix();
	Matrix4 camWorld = Matrix::Inverse(view);
	Vector3 rightAxis = Vector3(camWorld.GetColumn(0)); 
	Vector3 fwdAxis = Vector::Cross(Vector3(0, 1, 0), rightAxis);

	fwdAxis.y = 0.0f;
	fwdAxis = Vector::Normalise(fwdAxis);

	auto phys = selectionObject->GetPhysicsComponent();

	if(!phys)
		return;

	if (Window::GetKeyboard()->KeyDown(KeyCodes::UP)) 
		phys->GetPhysicsObject()->AddForce(fwdAxis);
	if (Window::GetKeyboard()->KeyDown(KeyCodes::DOWN)) 
		phys->GetPhysicsObject()->AddForce(-fwdAxis);
	if (Window::GetKeyboard()->KeyDown(KeyCodes::NEXT)) 
		phys->GetPhysicsObject()->AddForce(Vector3(0,-10,0));
}

void TutorialGame::InitCamera() {
	world->GetMainCamera().SetNearPlane(0.1f);
	world->GetMainCamera().SetFarPlane(500.0f);
	world->GetMainCamera().SetPitch(-15.0f);
	world->GetMainCamera().SetYaw(315.0f);
	world->GetMainCamera().SetPosition(Vector3(-60, 40, 60));
	lockedObject = nullptr;
}

void TutorialGame::InitWorld() 
{
	world->ClearAndErase();
	physics->Clear();
	InitGameExamples();
}

std::vector<Vector3> TutorialGame::GetVertices(Mesh* navigationMesh, int i)
{
	const SubMesh* subMesh = navigationMesh->GetSubMesh(i);
	const std::vector<unsigned int>& indices = navigationMesh->GetIndexData();
	const std::vector<Vector3>& positionData = navigationMesh->GetPositionData();
	std::vector<Vector3> vertices;

	for (size_t j = subMesh->start; j < subMesh->start + subMesh->count; j += 3) {
		unsigned int idx0 = indices[j];
		unsigned int idx1 = indices[j + 1];
		unsigned int idx2 = indices[j + 2];

		vertices.push_back(positionData[idx0]);
		vertices.push_back(positionData[idx1]);
		vertices.push_back(positionData[idx2]);
	}
	return vertices;
}


bool TutorialGame::RayCastNavWorld(Ray& r, float rayLength)
{

	Vector3 intersection = Vector3(0, 0, 0);
	Vector3 dir = Vector::Normalise(r.GetDirection());
	Vector3 pos = r.GetPosition();

	for (size_t i = 0; i < navigationMesh->GetSubMeshCount(); ++i) {
		const SubMesh* subMesh = navigationMesh->GetSubMesh(i);

		for (size_t j = subMesh->start; j < subMesh->start + subMesh->count; ++j) {
			Vector3 a, b, c;
			if (!navigationMesh->GetTriangle(j, a, b, c))
				continue;

			float t, u, v;
			if (RayIntersectsTriangle(pos, dir, a, b, c, t, u, v) && t <= rayLength) 
				return true;
		}
	}
	return false;
}


const bool DebugCubeTransforms = false;

void  TutorialGame::CalculateCubeTransformations(const std::vector<Vector3>& vertices, Vector3& position, Vector3& scale, Quaternion& rotation)
{
	Vector3 minBound(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
	Vector3 maxBound(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest());

	for (const auto& vertex : vertices) {
		minBound = Vector::Min(minBound, vertex);
		maxBound = Vector::Max(maxBound, vertex);
	}

	position = (minBound + maxBound) * 0.5f;
	Vector3 extent = maxBound - minBound;

	Vector3 a, b, c;
	a = vertices[1] - vertices[2];
	b = vertices[4] - vertices[5];
	c = vertices[8] - vertices[9];

	if (DebugCubeTransforms) {
		Debug::DrawLine(vertices[1], vertices[2], Vector4(1, 0, 0, 1));
		Debug::DrawLine(vertices[4], vertices[5], Vector4(0, 0, 1, 1));
		Debug::DrawLine(vertices[8], vertices[9], Vector4(0, 1, 0, 1));
	}

	extent = Vector3(Vector::Length(a),Vector::Length(b),Vector::Length(c));

	Vector3 localX = Vector::Normalise(a); 
	Vector3 localY = Vector::Normalise(b); 
	Vector3 localZ = -Vector::Normalise(c); 

	Matrix3 rotationMatrix = Matrix3();

	rotationMatrix.SetColumn(2, Vector4(localZ, 0));
	rotationMatrix.SetColumn(1, Vector4(localY, 0));
	rotationMatrix.SetColumn(0, Vector4(-localX, 0));

	rotation = Quaternion(rotationMatrix);
	scale = extent * 0.5f;
}


void TutorialGame::InitGameExamples() 
{	
	AddNavMeshToWorld(Vector3(0, 0, 0), Vector3(1, 1, 1));
}

void TutorialGame::InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius) {
	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 100.0f, z * rowSpacing);
			AddSphereToWorld(position, radius, 1.0f);
		}
	}
	AddFloorToWorld(Vector3(0, -2, 0));
}

bool TutorialGame::SelectObject() {
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::Q)) {
		inSelectionMode = !inSelectionMode;
	}
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
		if (Window::GetKeyboard()->KeyPressed(NCL::KeyCodes::L)) {
			if (selectionObject) {
				if (lockedObject == selectionObject) 
					lockedObject = nullptr;
				else 
					lockedObject = selectionObject;
			}
		}
	}

	return false;
}

void TutorialGame::MoveSelectedObject() {
	forceMagnitude += Window::GetMouse()->GetWheelMovement() * 100.0f;

	if (!selectionObject) 
		return;

	if (Window::GetMouse()->ButtonPressed(NCL::MouseButtons::Right)) 
	{
		Ray ray = CollisionDetection::BuildRayFromMouse(world->GetMainCamera());
		RayCollision closestCollision;

		if (world->Raycast(ray, closestCollision, true)) {
			if (closestCollision.node == selectionObject) {

				auto phys = selectionObject->GetPhysicsComponent();
				if (phys)
					phys->GetPhysicsObject()->AddForceAtPosition(ray.GetDirection() * forceMagnitude, closestCollision.collidedAt);
			}
		}
	}
}

void TutorialGame::DrawUIElements() {
	framerateDelay += 1;

	uiSystem->StartFrame();

	/*uiSystem->DrawDemo();*/
	if (framerateDelay > 10) {
		latestFramerate = Window::GetTimer().GetTimeDeltaSeconds();
		framerateDelay = 0;
	}
	uiSystem->DisplayFramerate(latestFramerate);
}



