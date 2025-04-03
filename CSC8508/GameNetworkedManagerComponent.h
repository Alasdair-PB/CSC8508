#pragma once

#ifndef GameNetworkedManagerComponent_H
#define GameNetworkedManagerComponent_H

#include "INetworkComponent.h"
#include "../CSC8508/GameManagerComponent.h"
	
namespace NCL::CSC8508 {


	struct GameManagerPacket : public INetworkPacket {
		bool successState;
		int casualties;
		int bankedCurrency;

		GameManagerPacket() {
			type = Component_Event;
			packetSubType = None;
			size = sizeof(GameManagerPacket) - sizeof(GamePacket);
		}
	};


	class GameNetworkedManagerComponent : public GameManagerComponent, public INetworkComponent {
	public:
		GameNetworkedManagerComponent(GameObject& gameObject, int quota, int terminationFee, int initAllowance, int objId, int ownId, int componentID, int pfabId, bool clientOwned)
			: GameManagerComponent(gameObject, quota, terminationFee, initAllowance), INetworkComponent(objId, ownId, componentID, pfabId, clientOwned) {
		}

		virtual std::unordered_set<std::type_index>& GetDerivedTypes() const override {
			static std::unordered_set<std::type_index> types = {
				std::type_index(typeid(IComponent)),
				std::type_index(typeid(GameManagerComponent)),
				std::type_index(typeid(INetworkComponent)),
			};
			return types;
		}

		bool ReadEventPacket(INetworkPacket& p) override {
			if (p.packetSubType == None) {
				GameManagerPacket* packet = &(GameManagerPacket&)p;

				casualties = packet->casualties;
				bankedCurrency = packet->bankedCurrency;
				successState = packet->successState;

				if (successState == Win)
					OnMissionSuccessful();
				else
					OnMissionFailure();
				return true;
			}
			return false;
		}

		void OnMissionEnd() override{
			if (!IsOwner()) return;
			GameManagerComponent::OnMissionEnd();
			SendManagerPacket();
		}

		void SendManagerPacket() {
			GameManagerPacket* packet = new GameManagerPacket();
			packet->casualties = this->casualties;
			packet->bankedCurrency = this->bankedCurrency;
			packet->successState = successState;
			SendEventPacket(packet);
		}

		void OnPauseEvent(PauseEvent* e) override {
			if (!IsOwner()) return;
			GameManagerComponent::OnPauseEvent(e);
		}

		void CheckPlayerInstance(DeathEvent* e) override {
			if (!IsOwner()) return;
			GameManagerComponent::CheckPlayerInstance(e);
		}
	};
}

#endif // 

