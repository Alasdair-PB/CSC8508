#include "GameNetworkedManagerComponent.h"
#include "ComponentManager.h"
#include "EventManager.h"
#include "../CSC8508/Legacy/PlayerComponent.h"

using namespace NCL::CSC8508;

void GameNetworkedManagerComponent::CheckPlayerInstance(DeathEvent* e) {
	if (!clientOwned) { return; }
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