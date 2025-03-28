#ifndef GAME_MANAGER_COMPONENT_H
#define GAME_MANAGER_COMPONENT_H

#include "IComponent.h"
#include <iostream>
#include "EventManager.h"
#include "../CSC8508/Legacy/PlayerComponent.h"
#include "DamageableComponent.h"
#include "PauseUI.h"
#include "UISystem.h"

namespace NCL::CSC8508 {

	class GameOverEvent : Event {
	public:
		GameOverEvent() {};
	};


	class GameManagerComponent : public IComponent, public EventListener<DeathEvent>, public EventListener<ExitEvent>, public EventListener<PauseEvent>, public EventListener<DebugEvent> {
	protected:
		int quota;
		int bankedCurrency;
		int terminationFee;
		int casualties = 0;

		inline static GameManagerComponent* instance = nullptr;

		void OnPauseEvent(PauseEvent* e);

		UI::PauseUI* pauseUI = new UI::PauseUI;

	public:
		GameManagerComponent(GameObject& gameObject)
			: IComponent(gameObject), quota(0), bankedCurrency(0), terminationFee(50) {
			instance = this;
			pauseUI->PushButtonElement(PauseReturnButton(), "Unpause");
			/*pauseUI->PushButtonElement(ExitButton(), "Exit");*/
		}

		~GameManagerComponent() = default;

		static GameManagerComponent* GetInstance() {
			return instance;
		}

		void OnAwake() override {
			EventManager::RegisterListener<DeathEvent>(this);
			EventManager::RegisterListener<ExitEvent>(this);
			EventManager::RegisterListener<PauseEvent>(this);
			EventManager::RegisterListener<DebugEvent>(this);
		}

		void Update(float dt) override {

		}

		
		void OnEvent(DeathEvent* e) override {
			CheckPlayerInstance(e);
		}

		void OnEvent(ExitEvent* e) override {
			OnExitEvent(e);
		}

		void OnEvent(PauseEvent* e) override {
			OnPauseEvent(e);
		}

		void OnEvent(DebugEvent* e) override {
			std::cout << "Debug event!" << std::endl;
		}


		virtual void CheckPlayerInstance(DeathEvent* e);

		virtual void OnExitEvent(ExitEvent* e);


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
			GameOverEvent* e = new GameOverEvent();
			EventManager::Call<GameOverEvent>(e);

		}

		void OnMissionFailure() {
			std::cout << "Mission failed! You lost!" << std::endl;
			GameOverEvent* e = new GameOverEvent();
			EventManager::Call<GameOverEvent>(e);
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

		std::function<CSC8508::PushdownState::PushdownResult()> PauseReturnButton() {
			std::function<CSC8508::PushdownState::PushdownResult()> func = [this]() -> CSC8508::PushdownState::PushdownResult {
				GameWorld::Instance().SetWorldPauseState(false);
				return CSC8508::PushdownState::PushdownResult::NoChange;
				};
			return func;
		}

		/*std::function<CSC8508::PushdownState::PushdownResult()> ExitButton() {
			std::function<CSC8508::PushdownState::PushdownResult()> func = [this]() -> CSC8508::PushdownState::PushdownResult {
				
				return CSC8508::PushdownState::PushdownResult::NoChange;
				};
			return func;
		}*/

	};
}
#endif
