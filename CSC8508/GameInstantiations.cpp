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
#include "TransformNetworkComponent.h"
#include "SightComponent.h"
#include "InventoryNetworkManagerComponent.h"
#include "InventoryManagerComponent.h"


float CantorPairing(int objectId, int index) { return (objectId + index) * (objectId + index + 1) / 2 + index;}

int GetUniqueId(int objectId, int& componentCount) {
	int unqiueId = CantorPairing(objectId, componentCount);
	componentCount++;
	return unqiueId;
}


void TutorialGame::Loaditem(const Vector3& position, NetworkSpawnData* spawnData) {
	std::string gameObjectPath = GetAssetPath("object_data.pfab");
	GameObject* myObjectToLoad = new GameObject();
	myObjectToLoad->Load(gameObjectPath);
	myObjectToLoad->GetTransform().SetPosition(myObjectToLoad->GetTransform().GetPosition() + position);
	myObjectToLoad->AddComponent<ItemComponent>(10);
	myObjectToLoad->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));
	if (spawnData)
	{	
		int componentIdCount = 0;
		TransformNetworkComponent* networkTransform = myObjectToLoad->AddComponent<TransformNetworkComponent>(
			spawnData->objId, spawnData->ownId, GetUniqueId(spawnData->objId, componentIdCount), spawnData->clientOwned);
	}
	world->AddGameObject(myObjectToLoad);
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

	player->GetTransform().SetScale(Vector3(meshSize, meshSize, meshSize)).SetPosition(position);
	player->SetLayerID(Layers::LayerID::Player);
	player->SetTag(Tags::Player);
	player->SetRenderObject(new RenderObject(&player->GetTransform(), playerMesh, basicTex, playerShader));

	StaminaComponent* stamina = player->AddComponent<StaminaComponent>(100, 100, 3);
	PlayerComponent* pc = player->AddComponent<PlayerComponent>();
	pc->SetBindingDash(KeyCodes::SHIFT, stamina);
	pc->SetBindingJump(KeyCodes::SPACE, stamina);
	pc->SetBindingInteract(KeyCodes::E);

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
		int unqiueId = GetUniqueId(spawnData->objId, componentIdCount);
		InputNetworkComponent* input = player->AddComponent<InputNetworkComponent>(
			controller, spawnData->objId, spawnData->ownId, GetUniqueId(spawnData->objId, componentIdCount), spawnData->clientOwned);

		InventoryNetworkManagerComponent* inventoryManager = player->AddComponent<InventoryNetworkManagerComponent>(2, carryOffset, dropOffset,
			spawnData->objId, spawnData->ownId, GetUniqueId(spawnData->objId, componentIdCount), spawnData->clientOwned);

		TransformNetworkComponent* networkTransform = player->AddComponent<TransformNetworkComponent>(
			spawnData->objId, spawnData->ownId, GetUniqueId(spawnData->objId, componentIdCount), spawnData->clientOwned);
		
		NetworkedListenerComponent* listenerComp = player->AddComponent<NetworkedListenerComponent>(
			world->GetMainCamera(), spawnData->objId, spawnData->ownId, GetUniqueId(spawnData->objId, componentIdCount), spawnData->clientOwned);

		AudioSourceComponent* sourceComp = player->AddComponent<AudioSourceComponent>();
		//sourceComp->LoadSound("pollo.mp3", 10.0f, FMOD_LOOP_NORMAL);
		sourceComp->setSoundCollection(*AudioEngine::Instance().GetSoundGroup(EntitySoundGroup::POLLO));
		sourceComp->RandomSound();

		if (spawnData->clientOwned)
			CameraComponent* cameraComponent = player->AddComponent<CameraComponent>(world->GetMainCamera(), *input);
		else
			listenerComp->SetPersistentSound(sourceComp->GetPersistentPair());
	}
	else {
		InventoryManagerComponent* inventoryManager = player->AddComponent<InventoryManagerComponent>(2, carryOffset, dropOffset);
		InputComponent* input = player->AddComponent<InputComponent>(controller);
		CameraComponent* cameraComponent = player->AddComponent<CameraComponent>(world->GetMainCamera(), *input);

		AudioListenerComponent* listenerComp = player->AddComponent<AudioListenerComponent>(world->GetMainCamera());
		listenerComp->RecordMic();
	}


	phys->SetPhysicsObject(new PhysicsObject(&player->GetTransform()));

	phys->GetPhysicsObject()->SetInverseMass(inverseMass);
	phys->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(player);

	return player;
}