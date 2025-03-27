#pragma once

#ifndef TIMERCOMPONENT_H
#define TIMERCOMPONENT_H

#include "IComponent.h"
#include <chrono>
#include "EventManager.h"

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
		}


		void Update(float dt) override {
			if (!isComplete) {
				remainingTime -= dt;
				HandleOverTime();
			}
		}

		void OnAwake() override {
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




	};
}

#endif

