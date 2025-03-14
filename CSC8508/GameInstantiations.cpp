#include "TutorialGame.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "INetworkComponent.h"
#include "InputNetworkComponent.h"
#include "TransformNetworkComponent.h"
#include "CameraComponent.h"
#include "MaterialManager.h"

using namespace NCL;
using namespace CSC8508;

GameObject* TutorialGame::AddNavMeshToWorld(const Vector3& position, Vector3 dimensions)
{
	navMesh = new NavigationMesh("smalltest.navmesh");
	GameObject* navMeshObject = new GameObject();
	Mesh* navigationMesh = MaterialManager::GetMesh("navMesh");
	Mesh* cubeMesh = MaterialManager::GetMesh("cube");
	Texture* basicTex = MaterialManager::GetTexture("basic");
	Shader* basicShader = MaterialManager::GetShader("basic");

	for (size_t i = 0; i < navigationMesh->GetSubMeshCount(); ++i)
	{
		if (navigationMesh->GetSubMesh(i)->count != 36)
			continue;

		std::vector<Vector3> vertices = GetVertices(navigationMesh, i);

		Vector3 dimensions, localPosition;
		Quaternion rotationMatrix;
		CalculateCubeTransformations(vertices, localPosition, dimensions, rotationMatrix);

		GameObject* colliderObject = new GameObject();
		OBBVolume* volume = new OBBVolume(dimensions);

		PhysicsComponent* phys = colliderObject->AddComponent<PhysicsComponent>();
		BoundsComponent* bounds = colliderObject->AddComponent<BoundsComponent>((CollisionVolume*)volume, phys);

		colliderObject->GetTransform().SetScale(dimensions * 2.0f).SetPosition(localPosition).SetOrientation(rotationMatrix);
		colliderObject->SetRenderObject(new RenderObject(&colliderObject->GetTransform(), cubeMesh, basicTex, basicShader));

		phys->SetPhysicsObject(new PhysicsObject(&colliderObject->GetTransform()));
		phys->GetPhysicsObject()->SetInverseMass(0);
		phys->GetPhysicsObject()->InitCubeInertia();
		colliderObject->SetLayerID(Layers::LayerID::Default);

		world->AddGameObject(colliderObject);
	}
	return navMeshObject;
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

	extent = Vector3(Vector::Length(a), Vector::Length(b), Vector::Length(c));

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

float CantorPairing(int objectId, int index) { return (objectId + index) * (objectId + index + 1) / 2 + index;}

int GetUniqueId(int objectId, int& componentCount) {
	int unqiueId = CantorPairing(objectId, componentCount);
	componentCount++;
	return unqiueId;
}

GameObject* TutorialGame::AddPlayerToWorld(const Vector3& position, NetworkSpawnData* spawnData) {
	float meshSize = 1.0f;
	float inverseMass = 0.5f;

	GameObject* player = new GameObject();
	CapsuleVolume* volume = new CapsuleVolume(0.5f, 0.5f);
	Mesh* capsuleMesh = MaterialManager::GetMesh("capsule");
	Shader* basicShader = MaterialManager::GetShader("basic");

	PlayerComponent* pc = player->AddComponent<PlayerComponent>();
	PhysicsComponent* phys = player->AddComponent<PhysicsComponent>();
	BoundsComponent* bounds = player->AddComponent<BoundsComponent>((CollisionVolume*)volume, phys);

	AudioListenerComponent* listenerComp = player->AddComponent<AudioListenerComponent>(world->GetMainCamera());

	int componentIdCount = 0;

	if (spawnData)
	{
		int unqiueId = GetUniqueId(spawnData->objId, componentIdCount);
		InputNetworkComponent* input = player->AddComponent<InputNetworkComponent>(
			controller, spawnData->objId, spawnData->ownId, GetUniqueId(spawnData->objId, componentIdCount), spawnData->clientOwned);

		TransformNetworkComponent* networkTransform = player->AddComponent<TransformNetworkComponent>(
			spawnData->objId, spawnData->ownId, GetUniqueId(spawnData->objId, componentIdCount), spawnData->clientOwned);

		if (spawnData->clientOwned) 
			CameraComponent* cameraComponent = player->AddComponent<CameraComponent>(world->GetMainCamera(), *input);
	}
	else {
		InputComponent* input = player->AddComponent<InputComponent>(controller);
		CameraComponent* cameraComponent = player->AddComponent<CameraComponent>(world->GetMainCamera(), *input);
	}

	player->GetTransform().SetScale(Vector3(meshSize, meshSize, meshSize)).SetPosition(position);
	player->SetLayerID(Layers::LayerID::Player);
	player->SetTag(Tags::Player);

	player->SetRenderObject(new RenderObject(&player->GetTransform(), capsuleMesh, nullptr, basicShader));
	player->GetRenderObject()->SetColour(Vector4(0, 0, 0, 1.0f));

	phys->SetPhysicsObject(new PhysicsObject(&player->GetTransform()));

	phys->GetPhysicsObject()->SetInverseMass(inverseMass);
	phys->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(player);

	std::cout << "Input Device : " << listenerComp->GetInputDeviceList()[0] << std::endl;
	std::cout << "Output Device : " << listenerComp->GetOutputDeviceList()[0] << std::endl;
	return player;
}

GameObject* TutorialGame::AddFloorToWorld(const Vector3& position)
{
	GameObject* floor = new GameObject();
	Vector3 floorSize = Vector3(200, 2, 200);
	AABBVolume* volume = new AABBVolume(floorSize);
	Mesh* cubeMesh = MaterialManager::GetMesh("cube");
	Texture* basicTex = MaterialManager::GetTexture("basic");
	Shader* basicShader = MaterialManager::GetShader("basic");


	PhysicsComponent* phys = floor->AddComponent<PhysicsComponent>();
	BoundsComponent* bounds = floor->AddComponent<BoundsComponent>((CollisionVolume*) volume, phys);

	floor->GetTransform().SetScale(floorSize * 2.0f).SetPosition(position);

	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, basicTex, basicShader));
	phys->SetPhysicsObject(new PhysicsObject(&floor->GetTransform()));

	phys->GetPhysicsObject()->SetInverseMass(0);
	phys->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(floor);
	return floor;
}

GameObject* TutorialGame::AddSphereToWorld(const Vector3& position, float radius, float inverseMass)
{
	GameObject* sphere = new GameObject();
	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius);
	Mesh* sphereMesh = MaterialManager::GetMesh("sphere");
	Texture* basicTex = MaterialManager::GetTexture("basic");
	Shader* basicShader = MaterialManager::GetShader("basic");


	PhysicsComponent* phys = sphere->AddComponent<PhysicsComponent>();
	BoundsComponent* bounds = sphere->AddComponent<BoundsComponent>((CollisionVolume*)volume, phys);

	bounds->SetBoundingVolume((CollisionVolume*)volume);
	sphere->GetTransform().SetScale(sphereSize).SetPosition(position);

	sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), sphereMesh, basicTex, basicShader));
	phys->SetPhysicsObject(new PhysicsObject(&sphere->GetTransform()));

	phys->GetPhysicsObject()->SetInverseMass(inverseMass);
	phys->GetPhysicsObject()->InitSphereInertia();
	phys->GetPhysicsObject()->SetRestitution(0.5f);

	world->AddGameObject(sphere);
	return sphere;
}

GameObject* TutorialGame::AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass) {
	GameObject* cube = new GameObject();
	OBBVolume* volume = new OBBVolume(dimensions);
	Mesh* cubeMesh = MaterialManager::GetMesh("cube");
	Texture* basicTex = MaterialManager::GetTexture("basic");
	Shader* basicShader = MaterialManager::GetShader("basic");

	PhysicsComponent* phys = cube->AddComponent<PhysicsComponent>();
	BoundsComponent* bounds = cube->AddComponent<BoundsComponent>((CollisionVolume*)volume, phys);

	bounds->SetBoundingVolume((CollisionVolume*)volume);
	cube->GetTransform().SetPosition(position).SetScale(dimensions * 2.0f);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	phys->SetPhysicsObject(new PhysicsObject(&cube->GetTransform()));

	phys->GetPhysicsObject()->SetInverseMass(inverseMass);
	phys->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(cube);
	return cube;
}