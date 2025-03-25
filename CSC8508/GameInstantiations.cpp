#include "TutorialGame.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "INetworkComponent.h"
#include "../AudioEngine/AudioSourceComponent.h"
#include "InputNetworkComponent.h"
#include "TransformNetworkComponent.h"
#include "StaminaComponent.h"
#include "CameraComponent.h"
#include "MaterialManager.h"
#include "../AudioEngine/AudioListenerComponent.h"
#include "../AudioEngine/NetworkedListenerComponent.h"
#include "../AudioEngine/AudioSourceComponent.h"
#include "AnimationComponent.h"
#include "MeshAnimation.h"
#include "Map/DungeonComponent.h"

using namespace NCL;
using namespace CSC8508;

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
	GameObject* sphereB = AddSphereToWorld(Vector3(0,5,0), 1, 0);
	
	player->AddChild(sphereB);

	CapsuleVolume* volume = new CapsuleVolume(0.5f, 0.5f);
	Mesh* capsuleMesh = MaterialManager::GetMesh("capsule");
	Shader* basicShader = MaterialManager::GetShader("basic");
	Texture* basicTex = MaterialManager::GetTexture("basic");

	StaminaComponent* stamina = player->AddComponent<StaminaComponent>(100,100, 3);
	PlayerComponent* pc = player->AddComponent<PlayerComponent>();
	pc->SetBindingDash(KeyCodes::SHIFT, stamina);
	pc->SetBindingJump(KeyCodes::SPACE, stamina);

	PhysicsComponent* phys = player->AddComponent<PhysicsComponent>();
	BoundsComponent* bounds = player->AddComponent<BoundsComponent>((CollisionVolume*)volume, phys);

	/*AudioSourceComponent * audio_src = player->AddComponent<AudioSourceComponent>(ChannelGroupType::SFX);
	audio_src->LoadSound("pollo.mp3", 10.0f, FMOD_LOOP_NORMAL);
	audio_src->PlaySound("pollo");
	audio_src->randomSounds(5);*/

	int componentIdCount = 0;

	if (spawnData)
	{
		int unqiueId = GetUniqueId(spawnData->objId, componentIdCount);
		InputNetworkComponent* input = player->AddComponent<InputNetworkComponent>(
			controller, spawnData->objId, spawnData->ownId, GetUniqueId(spawnData->objId, componentIdCount), spawnData->clientOwned);

		TransformNetworkComponent* networkTransform = player->AddComponent<TransformNetworkComponent>(
			spawnData->objId, spawnData->ownId, GetUniqueId(spawnData->objId, componentIdCount), spawnData->clientOwned);
		
		NetworkedListenerComponent* listenerComp = player->AddComponent<NetworkedListenerComponent>(
			world->GetMainCamera(), spawnData->objId, spawnData->ownId, GetUniqueId(spawnData->objId, componentIdCount), spawnData->clientOwned);
		listenerComp->RecordMic();

		AudioSourceComponent* sourceComp = player->AddComponent<AudioSourceComponent>();
		//sourceComp->setSoundCollection(*AudioEngine::Instance().GetSoundGroup(EntitySoundGroup::ENVIRONMENT));

		if (spawnData->clientOwned)
			CameraComponent* cameraComponent = player->AddComponent<CameraComponent>(world->GetMainCamera(), *input);
		else
			listenerComp->SetPersistentSound(sourceComp->GetPersistentPair());
	}
	else {
		InputComponent* input = player->AddComponent<InputComponent>(controller);
		CameraComponent* cameraComponent = player->AddComponent<CameraComponent>(world->GetMainCamera(), *input);

		AudioListenerComponent* listenerComp = player->AddComponent<AudioListenerComponent>(world->GetMainCamera());
		listenerComp->RecordMic();
	}

	player->GetTransform().SetScale(Vector3(meshSize, meshSize, meshSize)).SetPosition(position);
	player->SetLayerID(Layers::LayerID::Player);
	player->SetTag(Tags::Player);

	player->SetRenderObject(new RenderObject(&player->GetTransform(), capsuleMesh, basicTex, basicShader));
	phys->SetPhysicsObject(new PhysicsObject(&player->GetTransform()));

	phys->GetPhysicsObject()->SetInverseMass(inverseMass);
	phys->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(player);

	return player;
}

GameObject* TutorialGame::AddRoleTToWorld(const Vector3& position, float inverseMass)
{	
	GameObject* roleT = new GameObject();
	Vector3 size = Vector3(10.0f, 10.0f, 10.0f);
	CapsuleVolume* volume = new CapsuleVolume(4.0f, 2.5f);
	Mesh* roleTMesh = MaterialManager::GetMesh("Role_T");
	Texture* basicTex = MaterialManager::GetTexture("basic");
	Shader* animShader = MaterialManager::GetShader("anim");

	PhysicsComponent* phys = roleT->AddComponent<PhysicsComponent>();
	BoundsComponent* bounds = roleT->AddComponent<BoundsComponent>((CollisionVolume*)volume, phys);

	bounds->SetBoundingVolume((CollisionVolume*)volume);
	roleT->GetTransform().SetScale(size).SetPosition(position);

	roleT->SetRenderObject(new RenderObject(&roleT->GetTransform(), roleTMesh, basicTex, animShader));
	phys->SetPhysicsObject(new PhysicsObject(&roleT->GetTransform()));
	roleT->AddComponent<AnimationComponent>(new Rendering::MeshAnimation("Role_T.anm"));

	phys->GetPhysicsObject()->SetInverseMass(inverseMass);
	phys->GetPhysicsObject()->InitSphereInertia();
	phys->GetPhysicsObject()->SetRestitution(0.5f);

	world->AddGameObject(roleT);
	return roleT;
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

GameObject* TutorialGame::AddDungeonToWorld(Transform const& transform, DoorLocation const& entryPosition, int const roomCount) {
	auto* dungeon = new GameObject();
	dungeon->GetTransform() = transform;

	auto const* dc = dungeon->AddComponent<DungeonComponent>(entryPosition);
	dc->Generate(roomCount);

	world->AddGameObject(dungeon);
	return dungeon;
}
