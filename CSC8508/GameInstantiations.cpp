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
#include "TransformNetworkComponent.h"
#include "SightComponent.h"
#include "InventoryNetworkManagerComponent.h"
#include "InventoryManagerComponent.h"
#include "FallDamageComponent.h"
#include "DamageableComponent.h"

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

	AnimationComponent* animatior = player->AddComponent<AnimationComponent>(new Rendering::MeshAnimation("Walk.anm"));
	StaminaComponent* stamina = player->AddComponent<StaminaComponent>(100,100, 3);
	PlayerComponent* pc = player->AddComponent<PlayerComponent>();
	DamageableComponent* dc = player->AddComponent<DamageableComponent>(100, 100);
	FallDamageComponent* fdc = player->AddComponent<FallDamageComponent>(24,20);

	pc->SetBindingDash(KeyCodes::SHIFT, stamina);
	pc->SetBindingJump(KeyCodes::SPACE, stamina);
	pc->SetBindingInteract(KeyCodes::E);

	SightComponent* sight = player->AddComponent<SightComponent>();
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

		InventoryNetworkManagerComponent* inventoryManager = player->AddComponent<InventoryNetworkManagerComponent>(2, carryOffset, dropOffset,
			spawnData->objId, spawnData->ownId, GetUniqueId(spawnData->objId, componentIdCount), spawnData->clientOwned);

		TransformNetworkComponent* networkTransform = player->AddComponent<TransformNetworkComponent>(
			spawnData->objId, spawnData->ownId, GetUniqueId(spawnData->objId, componentIdCount), spawnData->clientOwned);
		
		NetworkedListenerComponent* listenerComp = player->AddComponent<NetworkedListenerComponent>(
			world->GetMainCamera(), spawnData->objId, spawnData->ownId, GetUniqueId(spawnData->objId, componentIdCount), spawnData->clientOwned);

		AudioSourceComponent* sourceComp = player->AddComponent<AudioSourceComponent>();
		//sourceComp->setSoundCollection(*AudioEngine::Instance().GetSoundGroup(EntitySoundGroup::ENVIRONMENT));

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