#include "TutorialGame.h"
#include "PhysicsObject.h"
#include "RenderObject.h"

#include "INetworkComponent.h"
#include "InputNetworkComponent.h"
#include "TransformNetworkComponent.h"
#include "CameraComponent.h"


using namespace NCL;
using namespace CSC8508;


GameObject* TutorialGame::AddNavMeshToWorld(const Vector3& position, Vector3 dimensions)
{
	navMesh = new NavigationMesh("smalltest.navmesh");
	GameObject* navMeshObject = new GameObject();

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

		phys->SetPhysicsObject(new PhysicsObject(&colliderObject->GetTransform(), bounds->GetBoundingVolume()));
		phys->GetPhysicsObject()->SetInverseMass(0);
		phys->GetPhysicsObject()->InitCubeInertia();
		colliderObject->SetLayerID(Layers::LayerID::Default);

		world->AddGameObject(colliderObject);
	}
	return navMeshObject;
}

float CantorPairing(int objectId, int index) { return (objectId + index) * (objectId + index + 1) / 2 + index;}

GameObject* TutorialGame::AddPlayerToWorld(const Vector3& position, NetworkSpawnData* spawnData) {
	float meshSize = 1.0f;
	float inverseMass = 0.5f;

	GameObject* player = new PlayerGameObject();
	CapsuleVolume* volume = new CapsuleVolume(0.5f, 0.5f);

	PhysicsComponent* phys = player->AddComponent<PhysicsComponent>();
	BoundsComponent* bounds = player->AddComponent<BoundsComponent>((CollisionVolume*)volume, phys);

	int componentIdCount = 0;

	if (spawnData)
	{
		int unqiueId = CantorPairing(spawnData->objId, componentIdCount);
		componentIdCount++;
		InputNetworkComponent* input = player->AddComponent<InputNetworkComponent>(
			&controller, spawnData->objId, spawnData->ownId, unqiueId, spawnData->clientOwned);

		unqiueId = CantorPairing(spawnData->objId, componentIdCount);
		componentIdCount++;
		TransformNetworkComponent* networkTransform = player->AddComponent<TransformNetworkComponent>(
			spawnData->objId, spawnData->ownId, unqiueId, spawnData->clientOwned);

		if (spawnData->clientOwned) 
			CameraComponent* cameraComponent = player->AddComponent<CameraComponent>(world->GetMainCamera(), *input);
	}
	else {
		InputComponent* input = player->AddComponent<InputComponent>(&controller);
		CameraComponent* cameraComponent = player->AddComponent<CameraComponent>(world->GetMainCamera(), *input);

	}

	player->GetTransform().SetScale(Vector3(meshSize, meshSize, meshSize)).SetPosition(position);
	player->SetLayerID(Layers::LayerID::Player);
	player->SetTag(Tags::Player);

	player->SetRenderObject(new RenderObject(&player->GetTransform(), capsuleMesh, nullptr, basicShader));
	player->GetRenderObject()->SetColour(Vector4(0, 0, 0, 1.0f));

	phys->SetPhysicsObject(new PhysicsObject(&player->GetTransform(), bounds->GetBoundingVolume()));

	phys->GetPhysicsObject()->SetInverseMass(inverseMass);
	phys->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(player);
	return player;
}

GameObject* TutorialGame::AddFloorToWorld(const Vector3& position)
{
	GameObject* floor = new GameObject();
	Vector3 floorSize = Vector3(200, 2, 200);
	AABBVolume* volume = new AABBVolume(floorSize);


	PhysicsComponent* phys = floor->AddComponent<PhysicsComponent>();
	BoundsComponent* bounds = floor->AddComponent<BoundsComponent>((CollisionVolume*) volume, phys);

	floor->GetTransform().SetScale(floorSize * 2.0f).SetPosition(position);

	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, basicTex, basicShader));
	phys->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), bounds->GetBoundingVolume()));

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

	PhysicsComponent* phys = sphere->AddComponent<PhysicsComponent>();
	BoundsComponent* bounds = sphere->AddComponent<BoundsComponent>((CollisionVolume*)volume, phys);

	bounds->SetBoundingVolume((CollisionVolume*)volume);
	sphere->GetTransform().SetScale(sphereSize).SetPosition(position);

	sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), sphereMesh, basicTex, basicShader));
	phys->SetPhysicsObject(new PhysicsObject(&sphere->GetTransform(), bounds->GetBoundingVolume()));

	phys->GetPhysicsObject()->SetInverseMass(inverseMass);
	phys->GetPhysicsObject()->InitSphereInertia();
	phys->GetPhysicsObject()->SetRestitution(0.5f);

	world->AddGameObject(sphere);
	return sphere;
}

GameObject* TutorialGame::AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass) {
	GameObject* cube = new GameObject();
	OBBVolume* volume = new OBBVolume(dimensions);

	PhysicsComponent* phys = cube->AddComponent<PhysicsComponent>();
	BoundsComponent* bounds = cube->AddComponent<BoundsComponent>((CollisionVolume*)volume, phys);

	bounds->SetBoundingVolume((CollisionVolume*)volume);
	cube->GetTransform().SetPosition(position).SetScale(dimensions * 2.0f);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	phys->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), bounds->GetBoundingVolume()));

	phys->GetPhysicsObject()->SetInverseMass(inverseMass);
	phys->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(cube);
	return cube;
}