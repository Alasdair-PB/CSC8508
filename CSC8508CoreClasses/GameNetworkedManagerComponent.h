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
		GameNetworkedManagerComponent(GameObject& gameObject, int objId, int ownId, int componentID, int pfabId, bool clientOwned)
			: GameManagerComponent(gameObject), INetworkComponent(objId, ownId, componentID, pfabId, clientOwned) {
		}

		virtual std::unordered_set<std::type_index>& GetDerivedTypes() const override {
			static std::unordered_set<std::type_index> types = {
				std::type_index(typeid(IComponent)),
				std::type_index(typeid(GameManagerComponent)),
				std::type_index(typeid(INetworkComponent)),
			};
			return types;
		}

		void OnAwake() override {
			EventManager::RegisterListener<DeathEvent>(this);
			EventManager::RegisterListener<ExitEvent>(this);
			EventManager::RegisterListener<PauseEvent>(this);
			EventManager::RegisterListener<DebugEvent>(this);
			EventManager::RegisterListener<OverTimeEvent>(this);
		}

		void Update(float dt) override {
		}


		bool ReadEventPacket(INetworkPacket& p) override {
			if (p.packetSubType == None) {

				GameManagerPacket* packet = &(GameManagerPacket&)p;

				casualties = packet->casualties;
				bankedCurrency = packet->bankedCurrency;
				successState = packet->successState;

				if (successState == Win)
					std::cout << "You Win " << std::endl;
				else if (successState == Loss)
					std::cout << "You Lose " << std::endl;
				else
					std::cout << "In Game " << std::endl;
				

				return true;
			}
			return false;
		}

		void OnEvent(DeathEvent* e) override {
			if (!clientOwned) return;
			CheckPlayerInstance(e);
		}

		void OnEvent(ExitEvent* e) override {
			if (!clientOwned) return;
			OnMissionEnd();
		}

		void OnEvent(PauseEvent* e) override {
			if (!clientOwned) return;
			OnPauseEvent(e);
		}

		void OnEvent(DebugEvent* e) override {
			if (!clientOwned) return;
			std::cout << "Debug event!" << std::endl;
		}

		void OnEvent(OverTimeEvent* e) override {
			if (!clientOwned) return;
			OnMissionEnd();
		}

		void OnMissionEnd() override{
			if (bankedCurrency >= quota) {
				OnMissionSuccessful();
			}
			else {
				OnMissionFailure();
			}

			if (successState == Win)
				std::cout << "You Win " << std::endl;
			else if (successState == Loss)
				std::cout << "You Lose " << std::endl;
			else
				std::cout << "In Game " << std::endl;
			SendManagerPacket();
		}

		void SendManagerPacket() {
			GameManagerPacket* packet = new GameManagerPacket();
			packet->casualties = this->casualties;
			packet->bankedCurrency = this->bankedCurrency;
			SendEventPacket(packet);
		}

	};
}

#endif // 

