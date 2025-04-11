#include "GameCore.h"
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
#include "DungeonComponent.h"
#include "TransformNetworkComponent.h"
#include "FullTransformNetworkComponent.h"
#include "SightComponent.h"
#include "InventoryNetworkManagerComponent.h"
#include "InventoryManagerComponent.h"
#include "FallDamageComponent.h"
#include "DamageableComponent.h"
#include "GameManagerComponent.h"
#include "DamageableNetworkComponent.h"
#include "StaminaNetworkComponent.h"
#include "TimerNetworkComponent.h"
#include "GameNetworkedManagerComponent.h"

float CantorPairing(int objectId, int index) { return (objectId + index) * (objectId + index + 1) / 2 + index;}

int GetUniqueId(int objectId, int& componentCount) {
	int unqiueId = CantorPairing(objectId, componentCount);
	componentCount++;
	return unqiueId;
}

GameObject* GameCore::Loaditem(const Vector3& position, NetworkSpawnData* spawnData) {
	std::string gameObjectPath = GetAssetPath("item.pfab");
	GameObject* myObjectToLoad = new GameObject();
	myObjectToLoad->Load(gameObjectPath);
	myObjectToLoad->GetTransform().SetPosition(position);
	myObjectToLoad->AddComponent<ItemComponent>(300, 50.0f);
	if (spawnData)
	{
		int pFabId = spawnData->pfab;
		int componentIdCount = 0;
		FullTransformNetworkComponent* networkTransform = myObjectToLoad->AddComponent<FullTransformNetworkComponent>(
			spawnData->objId, spawnData->ownId, GetUniqueId(spawnData->objId, componentIdCount), pFabId, spawnData->clientOwned);
	}
	myObjectToLoad->SetLayerID(Layers::LayerID::Player);
	world->AddGameObject(myObjectToLoad);
	return myObjectToLoad;
}

GameObject* GameCore::LoadGameManager(const Vector3& position, NetworkSpawnData* spawnData) {
	GameObject* gm = new GameObject();
	int quota = 500;
	int terminationFee = 60;
	int initAllowance = 300;
	int allowedTime = 300;

	if (spawnData)
	{
		int componentIdCount = 0;
		int pFabId = spawnData->pfab;
		int unqiueId = GetUniqueId(spawnData->objId, componentIdCount);
		gm->AddComponent<GameNetworkedManagerComponent>(quota, terminationFee, initAllowance, spawnData->objId,
			spawnData->ownId, GetUniqueId(spawnData->objId, componentIdCount), pFabId, spawnData->clientOwned);
		gm->AddComponent<TimerNetworkComponent>(allowedTime, spawnData->objId,
			spawnData->ownId, GetUniqueId(spawnData->objId, componentIdCount), pFabId, spawnData->clientOwned);
	}
	else {
		gm->AddComponent<TimerComponent>(allowedTime);
		gm->AddComponent<GameManagerComponent>(quota, terminationFee, initAllowance);
	}
	world->AddGameObject(gm);
	return gm;
}

GameObject* GameCore::AddPlayerToWorld(const Vector3& position, NetworkSpawnData* spawnData) {
	float meshSize = 0.25f;
	float inverseMass = 0.5f;

	GameObject* player = new GameObject();
	CapsuleVolume* volume = new CapsuleVolume(0.5f, 0.5f);

	Mesh* playerMesh = MaterialManager::GetMesh("astronaut");
	Texture* basicTex = MaterialManager::GetTexture("player");
	Shader* playerShader = MaterialManager::GetShader("anim");

	float carryOffset = 0.5f;
	float dropOffset = 3.0f;

	player->SetLayerID(Layers::Player);
	player->GetTransform().SetScale(Vector3(meshSize, meshSize, meshSize)).SetPosition(position);
	player->SetLayerID(Layers::LayerID::Player);
	player->AddTag(Tags::Player);
	player->SetRenderObject(new RenderObject(&player->GetTransform(), playerMesh, basicTex, playerShader));

	PlayerComponent* pc = player->AddComponent<PlayerComponent>();
	FallDamageComponent* fdc = player->AddComponent<FallDamageComponent>(24,20);
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
	StaminaComponent* stamina;

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

		stamina = player->AddComponent<StaminaNetworkComponent>(100, 100, 7, spawnData->objId,
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
		stamina = player->AddComponent<StaminaComponent>(100, 100, 7);
		AudioListenerComponent* listenerComp = player->AddComponent<AudioListenerComponent>(world->GetMainCamera());
		listenerComp->RecordMic();
		pc->SetBindingPause(controller->GetButtonHashId("Pause"));
	}

	pc->SetBindingDash(controller->GetButtonHashId("Dash"), stamina);
	pc->SetBindingJump(controller->GetButtonHashId("Jump"), stamina);
	pc->SetBindingInteract(controller->GetButtonHashId("Interact"));
	pc->SetBindingDebug(controller->GetButtonHashId("Debug"));

	phys->SetPhysicsObject(new PhysicsObject(&player->GetTransform()));
	phys->GetPhysicsObject()->SetInverseMass(inverseMass);
	phys->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(player);

	return player;
}

GameObject* GameCore::AddDungeonToWorld(Transform const& transform, DoorLocation const& entryPosition, int const roomCount) {
	auto* dungeon = new GameObject();
	dungeon->GetTransform() = transform;
	DungeonComponent* dc = dungeon->AddComponent<DungeonComponent>(entryPosition, roomCount);
	world->AddGameObject(dungeon);
	return dungeon;
}