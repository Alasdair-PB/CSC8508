#ifndef GAME_MANAGER_COMPONENT_H
#define GAME_MANAGER_COMPONENT_H

#include "IComponent.h"
#include <iostream>
#include "EventManager.h"
#include "../CSC8508/Legacy/PlayerComponent.h"
#include "DamageableComponent.h"
#include "PauseUI.h"
#include "FramerateUI.h"
#include "GameOverUI.h"
#include "GameVictoryUI.h"
#include "UISystem.h"
#include "TimerComponent.h"

namespace NCL::CSC8508 {

	class GameOverEvent : Event {
	public:
		GameOverEvent() {};
	};

	class GameManagerComponent : public IComponent, public EventListener<DeathEvent>, public EventListener<ExitEvent>, public EventListener<PauseEvent>, public EventListener<DebugEvent>, public EventListener<OverTimeEvent> {
	protected:
		int quota;
		int bankedCurrency = 0;
		int terminationFee;
		int casualties = 0;
		int framerateDelay;
		int successState = InGame;
		enum SuccessStates { InGame, Loss, Win};

		bool debugMode = false;
		inline static GameManagerComponent* instance = nullptr;

		UI::PauseUI* pauseUI = new UI::PauseUI;
		UI::FramerateUI* framerate = new UI::FramerateUI;
		UI::GameOverUI* gameOverUI = new UI::GameOverUI;
		UI::GameVictoryUI* gameVicUI = new UI::GameVictoryUI;


		void OnEvent(DeathEvent* e) override {
			CheckPlayerInstance(e);
		}

		void OnEvent(ExitEvent* e) override {
			OnMissionEnd();
		}

		void OnEvent(PauseEvent* e) override {
			OnPauseEvent(e);
		}

		void OnEvent(DebugEvent* e) override {
			if (debugMode == false) {
				debugMode = true;
				UI::UISystem::GetInstance()->PushNewStack(framerate->frameUI, "Framerate");
			}
			else {
				debugMode = false;
				UI::UISystem::GetInstance()->RemoveStack("Framerate");
			}
		}

		virtual void OnEvent(OverTimeEvent* e) override {
			OnMissionEnd();
		}

	public:

		GameManagerComponent(GameObject& gameObject)
			: IComponent(gameObject), quota(0), bankedCurrency(0), terminationFee(9) {
			instance = this;
			pauseUI->PushButtonElement(PauseReturnButton(), "Unpause");
			/*pauseUI->PushButtonElement(ExitButton(), "Exit");*/
		}

		~GameManagerComponent() = default;

		static GameManagerComponent* GetInstance() {
			return instance;
		}

		virtual void OnAwake() override {
			EventManager::RegisterListener<DeathEvent>(this);
			EventManager::RegisterListener<ExitEvent>(this);
			EventManager::RegisterListener<PauseEvent>(this);
			EventManager::RegisterListener<DebugEvent>(this);
			EventManager::RegisterListener<OverTimeEvent>(this);
		}

		void Update(float dt) override {
			framerateDelay += 1;

			if (framerateDelay > 10) {
				framerate->UpdateFramerate(Window::GetTimer().GetTimeDeltaSeconds());
				framerateDelay = 0;
			}
		}

		virtual void OnPauseEvent(PauseEvent* e);
		virtual void CheckPlayerInstance(DeathEvent* e);

		virtual void OnMissionEnd() {
			if (bankedCurrency >= quota)
				OnMissionSuccessful();
			else
				OnMissionFailure();
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

		void OnMissionFeedBack() {
			GameWorld::Instance().ToggleWorldPauseState();
		}

		void OnMissionSuccessful() {
			successState = Win;
			GameOverEvent* e = new GameOverEvent();
			EventManager::Call<GameOverEvent>(e);
			UI::UISystem::GetInstance()->PushNewStack(gameVicUI->gameOverUI, "Game victory");
			gameVicUI->PushElement(GameOverCurrency());
			OnMissionFeedBack();
		}

		void OnMissionFailure() {
			successState = Loss;
			GameOverEvent* e = new GameOverEvent();
			EventManager::Call<GameOverEvent>(e);
			UI::UISystem::GetInstance()->PushNewStack(gameOverUI->gameOverUI, "Game Over");
			gameOverUI->PushElement(GameOverCurrency());
			OnMissionFeedBack();
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

		void UpdateFramerate(float f) {
			framerate->UpdateFramerate(f);
		}

		std::function<CSC8508::PushdownState::PushdownResult()> PauseReturnButton() {
			std::function<CSC8508::PushdownState::PushdownResult()> func = [this]() -> CSC8508::PushdownState::PushdownResult {
				GameWorld::Instance().ToggleWorldPauseState();
				return CSC8508::PushdownState::PushdownResult::NoChange;
				};
			return func;
		}

		std::function<CSC8508::PushdownState::PushdownResult()> GameOverCurrency() {
			std::function<CSC8508::PushdownState::PushdownResult()> func = [this]() -> CSC8508::PushdownState::PushdownResult {
				std::string text = "You earned: " + std::to_string(bankedCurrency) + " credits!";
				ImGui::Text(text.c_str());
				return CSC8508::PushdownState::PushdownResult::NoChange;
				};
			return func;
		}
	};
}
#endif
