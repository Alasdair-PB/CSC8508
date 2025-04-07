#pragma once

#include "GameManagerComponent.h"
#include "ComponentManager.h"
#include "EventManager.h"
#include "PlayerComponent.h"
using namespace NCL::CSC8508;

void GameManagerComponent::CheckPlayerInstance(DeathEvent* e) {
	GameObject* object = &e->GetGameObject();
	GameManagerComponent* thisObj = this;

	ComponentManager::OperateOnBufferContents<PlayerComponent>(
		[&object, &thisObj](PlayerComponent* o) {
			if (&o->GetGameObject() == object) {
				thisObj->IncrementCasualties();
				thisObj->TryRespawnPlayer();
				// Send event to respawn player if above is true
			}
		}
	);
}

int GameManagerComponent::GetBankedTotal() {
	int* bankSum = new int();
	ComponentManager::OperateOnBufferContents<InventoryManagerComponent>(
		[bankSum](InventoryManagerComponent* o) { *bankSum += o->GetDepositedSum(); });
	ComponentManager::OperateOnBufferContents<InventoryNetworkManagerComponent>(
		[bankSum](InventoryNetworkManagerComponent* o) { *bankSum += o->GetDepositedSum(); });
	int val = *bankSum;
	delete bankSum;
	return val;
}

void GameManagerComponent::OnPauseEvent(PauseEvent* e) {
	GameWorld::Instance().ToggleWorldPauseState();
	UI::UISystem::GetInstance()->PushNewStack(pauseUI->pauseUI, "Pause");
}
