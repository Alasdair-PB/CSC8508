#include "TutorialGame.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "../AudioEngine/AudioSourceComponent.h"
#include "InputNetworkComponent.h"
#include "StaminaComponent.h"
#include "CameraComponent.h"
#include "MaterialManager.h"
#include "../AudioEngine/AudioListenerComponent.h"
#include "../AudioEngine/NetworkedListenerComponent.h"
#include "AnimationComponent.h"
#include "MeshAnimation.h"
#include "Map/DungeonComponent.h"
#include "TransformNetworkComponent.h"
#include "FullTransformNetworkComponent.h"
#include "SightComponent.h"
#include "InventoryNetworkManagerComponent.h"
#include "InventoryManagerComponent.h"
#include "FallDamageComponent.h"
#include "DamageableComponent.h"
#include "GameManagerComponent.h"
#include "DamageableNetworkComponent.h"

float CantorPairing(int objectId, int index) { return (objectId + index) * (objectId + index + 1) / 2 + index;}

int GetUniqueId(int objectId, int& componentCount) {
	int unqiueId = CantorPairing(objectId, componentCount);
	componentCount++;
	return unqiueId;
}


GameObject* TutorialGame::Loaditem(const Vector3& position, NetworkSpawnData* spawnData) {
	std::string gameObjectPath = GetAssetPath("object_data.pfab");
	GameObject* myObjectToLoad = new GameObject();
	myObjectToLoad->Load(gameObjectPath);
	myObjectToLoad->GetTransform().SetPosition(position);
	myObjectToLoad->AddComponent<ItemComponent>(10);
	myObjectToLoad->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));
	if (spawnData)
	{
		int pFabId = spawnData->pfab;
		int componentIdCount = 0;
		FullTransformNetworkComponent* networkTransform = myObjectToLoad->AddComponent<FullTransformNetworkComponent>(
			spawnData->objId, spawnData->ownId, GetUniqueId(spawnData->objId, componentIdCount), pFabId, spawnData->clientOwned);
	}
	world->AddGameObject(myObjectToLoad);

	return myObjectToLoad;
}



GameObject* TutorialGame::LoadGameManager(const Vector3& position, NetworkSpawnData* spawnData) {
	GameObject* myObjectToLoad = new GameObject();
#
	myObjectToLoad->AddComponent<GameManagerComponent>();/*
	if (spawnData)
	{
		int pFabId = spawnData->pfab;
		int componentIdCount = 0;
		FullTransformNetworkComponent* networkTransform = myObjectToLoad->AddComponent<FullTransformNetworkComponent>(
			spawnData->objId, spawnData->ownId, GetUniqueId(spawnData->objId, componentIdCount), pFabId, spawnData->clientOwned);
	}*/
	world->AddGameObject(myObjectToLoad);

	return myObjectToLoad;
}

GameObject* TutorialGame::LoadDropZone(const Vector3& position, Vector3 dimensions, Tag tag) {

	//std::string gameObjectPath = GetAssetPath("object_data.pfab");
	GameObject* dropZone = new GameObject();
	//myObjectToLoad->Load(gameObjectPath);

	OBBVolume* volume = new OBBVolume(dimensions);
	Mesh* cubeMesh = MaterialManager::GetMesh("cube");
	Texture* basicTex = MaterialManager::GetTexture("basic");
	Shader* basicShader = MaterialManager::GetShader("basic");

	PhysicsComponent* phys = dropZone->AddComponent<PhysicsComponent>();
	BoundsComponent* bounds = dropZone->AddComponent<BoundsComponent>((CollisionVolume*)volume, phys);

	bounds->AddToIgnoredLayers(Layers::LayerID::Player);
	bounds->SetBoundingVolume((CollisionVolume*)volume);
	dropZone->GetTransform().SetPosition(position - Vector3(0,5,0)).SetScale(dimensions * 2.0f);

	dropZone->SetRenderObject(new RenderObject(&dropZone->GetTransform(), cubeMesh, basicTex, basicShader));
	phys->SetPhysicsObject(new PhysicsObject(&dropZone->GetTransform()));
	phys->GetPhysicsObject()->SetInverseMass(0);
	phys->GetPhysicsObject()->InitCubeInertia();
	dropZone->SetTag(tag);
	if (tag == Tags::DropZone)
		dropZone->GetRenderObject()->SetColour(Vector4(0, 1, 0, 0.3f));
	else if (tag == Tags::Exit)
		dropZone->GetRenderObject()->SetColour(Vector4(1, 0, 0, 0.3f));
	else if (tag == Tags::DepositZone)
		dropZone->GetRenderObject()->SetColour(Vector4(0, .2, 1, 0.3f));
	else
		dropZone->GetRenderObject()->SetColour(Vector4(1, 0, 1, 0.3f));

	world->AddGameObject(dropZone);
	return dropZone;
}

GameObject* TutorialGame::AddPlayerToWorld(const Vector3& position, NetworkSpawnData* spawnData) {
	float meshSize = 0.25f;
	float inverseMass = 0.5f;

	GameObject* player = new GameObject();
	CapsuleVolume* volume = new CapsuleVolume(0.5f, 0.5f);

	Mesh* playerMesh = MaterialManager::GetMesh("player");
	Texture* basicTex = MaterialManager::GetTexture("player");
	Shader* playerShader = MaterialManager::GetShader("anim");

	float carryOffset = 0.5f;
	float dropOffset = 3.0f;

	player->SetLayerID(Layers::Player);
	player->GetTransform().SetScale(Vector3(meshSize, meshSize, meshSize)).SetPosition(position);
	player->SetLayerID(Layers::LayerID::Player);
	player->SetTag(Tags::Player);
	player->SetRenderObject(new RenderObject(&player->GetTransform(), playerMesh, basicTex, playerShader));

	StaminaComponent* stamina = player->AddComponent<StaminaComponent>(100, 100, 3);
	PlayerComponent* pc = player->AddComponent<PlayerComponent>();
	FallDamageComponent* fdc = player->AddComponent<FallDamageComponent>(24,20);

	pc->SetBindingDash(controller->GetButtonHashId("Dash"), stamina);
	pc->SetBindingJump(controller->GetButtonHashId("Jump"), stamina);
	pc->SetBindingInteract(controller->GetButtonHashId("Interact"));
	pc->SetBindingDebug(controller->GetButtonHashId("Debug"));
	DamageableComponent* dc = player->AddComponent<DamageableComponent>(100, 100);

	AnimationComponent* animator = player->AddComponent<AnimationComponent>();

	AnimState* walk = new AnimState(new Rendering::MeshAnimation("Walk.anm"));
	AnimState* stance = new AnimState(new Rendering::MeshAnimation("Stance.anm"));
	AnimState* onjump = new AnimState(new Rendering::MeshAnimation("OnJump.anm"), false);
	AnimState* jumping = new AnimState(new Rendering::MeshAnimation("Jumping.anm"));
	AnimState* jumpland = new AnimState(new Rendering::MeshAnimation("JumpLand.anm"), false);

	animator->AddState(walk);
	animator->AddState(onjump);
	animator->AddState(jumping);
	animator->AddState(jumpland);
	animator->AddState(stance);

	animator->AddTransition(new IStateTransition(stance, walk, [pc]()->bool {
		return pc->IsMoving();
		}));
	animator->AddTransition(new IStateTransition(walk, stance, [pc]()->bool {
		return !pc->IsMoving();
		}));
	animator->AddTransition(new IStateTransition(stance, onjump, [pc]()->bool {
		return pc->IsJumping();
		}));
	animator->AddTransition(new IStateTransition(walk, onjump, [pc]()->bool {
		return pc->IsJumping();
		}));
	animator->AddTransition(new IStateTransition(onjump, jumping, [onjump]()->bool {
		return onjump->IsComplete();
		}));
	animator->AddTransition(new IStateTransition(jumping, jumpland, [pc]()->bool {
		return pc->IsGrounded();
		}));
	animator->AddTransition(new IStateTransition(jumpland , stance, [jumpland]()->bool {
		return jumpland->IsComplete();
		}));

	SightComponent* sight = player->AddComponent<SightComponent>();
	PhysicsComponent* phys = player->AddComponent<PhysicsComponent>();
	BoundsComponent* bounds = player->AddComponent<BoundsComponent>((CollisionVolume*)volume, phys);

	int componentIdCount = 0;

	if (spawnData)
	{
		int pFabId = spawnData->pfab;
		int unqiueId = GetUniqueId(spawnData->objId, componentIdCount);
		InputNetworkComponent* input = player->AddComponent<InputNetworkComponent>(
			controller, spawnData->objId, spawnData->ownId, GetUniqueId(spawnData->objId, componentIdCount), pFabId, spawnData->clientOwned);

		InventoryNetworkManagerComponent* inventoryManager = player->AddComponent<InventoryNetworkManagerComponent>(2, carryOffset, dropOffset,
			spawnData->objId, spawnData->ownId, GetUniqueId(spawnData->objId, componentIdCount), pFabId, spawnData->clientOwned);

		TransformNetworkComponent* networkTransform = player->AddComponent<TransformNetworkComponent>(
			spawnData->objId, spawnData->ownId, GetUniqueId(spawnData->objId, componentIdCount), pFabId, spawnData->clientOwned);
		
		NetworkedListenerComponent* listenerComp = player->AddComponent<NetworkedListenerComponent>(
			world->GetMainCamera(), spawnData->objId, spawnData->ownId, GetUniqueId(spawnData->objId, componentIdCount), pFabId, spawnData->clientOwned);

		AudioSourceComponent* sourceComp = player->AddComponent<AudioSourceComponent>();

		sourceComp->setSoundCollection(*AudioEngine::Instance().GetSoundGroup(EntitySoundGroup::PLAYER));
		sourceComp->PlaySound("join");

		DamageableNetworkComponent* dc = player->AddComponent<DamageableNetworkComponent>(100, 100, spawnData->objId,
			spawnData->ownId, GetUniqueId(spawnData->objId, componentIdCount), pFabId, spawnData->clientOwned);

		if (spawnData->clientOwned)
			CameraComponent* cameraComponent = player->AddComponent<CameraComponent>(world->GetMainCamera(), *input);
		else
			listenerComp->SetPersistentSound(sourceComp->GetPersistentPair());
	}
	else {
		InventoryManagerComponent* inventoryManager = player->AddComponent<InventoryManagerComponent>(2, carryOffset, dropOffset);
		InputComponent* input = player->AddComponent<InputComponent>(controller);
		CameraComponent* cameraComponent = player->AddComponent<CameraComponent>(world->GetMainCamera(), *input);
		DamageableComponent* dc = player->AddComponent<DamageableComponent>(100, 100);
		AudioListenerComponent* listenerComp = player->AddComponent<AudioListenerComponent>(world->GetMainCamera());
		listenerComp->RecordMic();
		pc->SetBindingPause(controller->GetButtonHashId("Pause"));
	}


	phys->SetPhysicsObject(new PhysicsObject(&player->GetTransform()));

	phys->GetPhysicsObject()->SetInverseMass(inverseMass);
	phys->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(player);

	return player;
}

GameObject* TutorialGame::AddDungeonToWorld(Transform const& transform, DoorLocation const& entryPosition, int const roomCount) {
	auto* dungeon = new GameObject();
	dungeon->GetTransform() = transform;

	auto const* dc = dungeon->AddComponent<DungeonComponent>(entryPosition);
	dc->Generate(roomCount);

	world->AddGameObject(dungeon);
	return dungeon;
}