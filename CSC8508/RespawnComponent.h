#pragma once
#ifndef RESPAWN_COMPONENT_H
#define RESPAWN_COMPONENT_H

#include "IComponent.h"
#include "GameManagerComponent.h"
#include "GameObject.h"
#include <vector>
#include <random>

namespace NCL::CSC8508 {
	class RespawnComponent : public IComponent {
	private:
		std::vector<Vector3> spawnPoints;

	public:
		RespawnComponent(GameObject& gameObject, const std::vector<Vector3>& points = {})
			: IComponent(gameObject), spawnPoints(points) {}

		void SetSpawnPoints(const std::vector<Vector3>& points) {
			spawnPoints = points;
		}

		const std::vector<Vector3>& GetSpawnPoints() const {
			return spawnPoints;
		}

		void DespawnPlayer() {
			GetGameObject().SetEnabled(false);

			auto gm = GameManagerComponent::GetInstance();
			if (gm && gm->TryRespawnPlayer()) {
				RespawnGameObject();
			}
		}

		void RespawnGameObject() {
			GetGameObject().SetEnabled(true);
			if (!spawnPoints.empty()) {
				int index = rand() % spawnPoints.size();
				GetGameObject().GetTransform().SetPosition(spawnPoints[index]);
			}
		}
	};
}
#endif
