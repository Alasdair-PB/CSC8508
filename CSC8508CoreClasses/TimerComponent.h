#pragma once

#ifndef TIMERCOMPONENT_H
#define TIMERCOMPONENT_H

#include "IComponent.h"
#include <chrono>

namespace NCL::CSC8508
{
	class TimerComponent : public IComponent
	{
	public:

		TimerComponent(GameObject& gameObject) : IComponent(gameObject) {}

		~TimerComponent() {
			std::cout << "TimerComponent destroyed" << std::endl;
		}


		void Update(float dt) override {
			if (!isComplete && !isPaused) {
				auto now = std::chrono::high_resolution_clock::now();
				std::chrono::duration<float> elapsedTime = now - lastUpdateTime;
				remainingTime -= elapsedTime.count();
				lastUpdateTime = now;

				if (remainingTime <= 0.0f) {
					remainingTime = 0.0f;
					isComplete = true;
				}
			}
		}

		void OnAwake() override {
			isComplete = true;
			isPaused = false;
			remainingTime = -1.0f;
		}

		void SetTime(float time) {
			remainingTime = time;
			lastUpdateTime = std::chrono::high_resolution_clock::now();
		}

		void Pause() {
			isPaused = true;
		}

		void Unpause() {
			if (!isPaused) return;
			isPaused = false;
			lastUpdateTime = std::chrono::high_resolution_clock::now();
		}

		void TogglePause() {
			isPaused ? Unpause() : Pause();
		}

	private:

		bool isComplete;
		bool isPaused;

		float remainingTime;
		std::chrono::high_resolution_clock::time_point lastUpdateTime;





	};
}

#endif

