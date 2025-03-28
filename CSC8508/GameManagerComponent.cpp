#pragma once

#include "GameManagerComponent.h"
#include "ComponentManager.h"
#include "EventManager.h"
#include "Legacy/PlayerComponent.h"

using namespace NCL::CSC8508;

void GameManagerComponent::CheckPlayerInstance(DeathEvent* e) {
	GameObject* object = &e->GetGameObject();
	GameManagerComponent* thisObj = this;

	ComponentManager::OperateOnBufferContents<PlayerComponent>(
		[&object, &thisObj](PlayerComponent* o) {
			if (&o->GetGameObject() == object) {
				thisObj->IncrementCasualties();
				thisObj->TryRespawnPlayer();
			}
		}
	);
}

int GameManagerComponent::GetTotalQuota() {
	int amount = 0;
	ComponentManager::OperateOnBufferContents<InventoryManagerComponent>(
		[&amount](InventoryManagerComponent* o) {
			amount += o->GetWallet();
		}
	);

	return amount;
}


void GameManagerComponent::OnExitEvent(ExitEvent* e) {
	std::cout << "Mission ended! Game Over!" << std::endl;
	if (bankedCurrency >= quota) {
		OnMissionSuccessful();
	}
	else {
		OnMissionFailure();
	}
}

void GameManagerComponent::OnPauseEvent(PauseEvent* e) {
	GameWorld::Instance().ToggleWorldPauseState(true);
	UI::UISystem::GetInstance()->PushNewStack(pauseUI->pauseUI, "Pause");
}
