#pragma once

#ifndef TIMERCOMPONENT_H
#define TIMERCOMPONENT_H

#include "IComponent.h"
#include <chrono>
#include "EventManager.h"
#include "TimerUI.h"
#include "UISystem.h"

namespace NCL::CSC8508
{

	struct OverTimeEvent : public Event {
		OverTimeEvent() {
		}
	};

	class TimerComponent : public IComponent
	{
	public:

		TimerComponent(GameObject& gameObject, float timeRemaining) : IComponent(gameObject) {
			isComplete = false;
			remainingTime = timeRemaining;
		}

		~TimerComponent() {
			UI::UISystem::GetInstance()->RemoveStack("Timer");
		}

		virtual void OnAwake() override
		{
			InititUI();
		}

		void InititUI() {
			UI::UISystem::GetInstance()->PushNewStack(timerUI->timerUI, "Timer");
			timerUI->UpdateTimer(remainingTime);
		}

		void Update(float dt) override {
			if (!isComplete) {
				remainingTime -= dt;
				HandleOverTime();
				UpdateUI();
			}
		}

		void UpdateUI() {
			timerUI->UpdateTimer(remainingTime);
		}

		void SetTime(float time) {
			remainingTime = time;
		}

		float GetRemainingTime() const {
			return remainingTime;
		}

		bool IsComplete() const {
			return isComplete;
		}


		void HandleOverTime() {
			if (remainingTime <= 0.0f) {
				remainingTime = 0.0f;
				isComplete = true;
				OverTimeEvent* e = new OverTimeEvent();
				EventManager::Call<OverTimeEvent>(e);
				delete e;
			}
		}

	protected:

		bool isComplete;
		float remainingTime;

		UI::TimerUI* timerUI = new UI::TimerUI;


	};
}

#endif

