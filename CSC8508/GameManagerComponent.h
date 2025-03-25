#pragma once
#ifndef GAME_MANAGER_COMPONENT_H
#define GAME_MANAGER_COMPONENT_H

#include "IComponent.h"
#include <iostream>
#include <vector>

namespace NCL::CSC8508 {
	class GameManagerComponent : public IComponent {
	private:
		int quota;
		int bankedCurrency;
		int terminationFee;
		std::vector<Vector3> spawnPoints;

		static GameManagerComponent* instance;

	public:
		GameManagerComponent(GameObject& gameObject)
			: IComponent(gameObject), quota(0), bankedCurrency(0), terminationFee(50) {
			instance = this;
		}

		static GameManagerComponent* GetInstance() {
			return instance;
		}

		void OnMissionEnd() {
			if (bankedCurrency >= quota) {
				OnMissionSuccessful();
			}
			else {
				OnMissionFailure();
			}
		}

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

		void SetSpawnPoints(const std::vector<Vector3>& points) {
			spawnPoints = points;
		}

		const std::vector<Vector3>& GetSpawnPoints() const {
			return spawnPoints;
		}
	};
}
#endif
