#ifndef GAME_MANAGER_COMPONENT_H
#define GAME_MANAGER_COMPONENT_H

#include "IComponent.h"
#include <iostream>
#include "EventManager.h"
#include "DamageableComponent.h"

namespace NCL::CSC8508 {
	class GameManagerComponent : public IComponent, public EventListener<DeathEvent> {
	protected:
		int quota;
		int bankedCurrency;
		int terminationFee;
		int casualties = 0;

		inline static GameManagerComponent* instance = nullptr;

	public:
		GameManagerComponent(GameObject& gameObject)
			: IComponent(gameObject), quota(0), bankedCurrency(0), terminationFee(50) {
			instance = this;
		}

		static GameManagerComponent* GetInstance() {
			return instance;
		}

		void OnAwake() override {
			EventManager::RegisterListener<DeathEvent>(this);
		}

		void Update(float dt) override {

		}

		
		void OnEvent(DeathEvent* e) override {
			CheckPlayerInstance(e);
		}

		virtual void CheckPlayerInstance(DeathEvent* e);


		void OnMissionEnd() {
			std::cout << "Mission ended! Game Over!" << std::endl;
			if (bankedCurrency >= quota) {
				OnMissionSuccessful();
			}
			else {
				OnMissionFailure();
			}
		}

		int GetTotalQuota();

		bool TryRespawnPlayer() {
			bankedCurrency -= terminationFee;
			if (bankedCurrency < 0) {
				OnMissionEnd();
				return false;
			}
			return true;
		}

		void OnMissionSuccessful() {
			std::cout << "Mission successful! Victory!" << std::endl;
		}

		void OnMissionFailure() {
			std::cout << "Mission failed! You lost!" << std::endl;
		}

		void AddToBank(int amount) {
			bankedCurrency += amount;
		}

		int GetBankedCurrency() const {
			return bankedCurrency;
		}

		void IncrementCasualties() {
			casualties++;
		}


	};
}
#endif
