#pragma once

#ifndef TIMERNETWORKCOMPONENT_H
#define TIMERNETWORKCOMPONENT_H

#include "TimerComponent.h"
#include "INetworkComponent.h"

namespace NCL::CSC8508
{
	struct TimerPacket : INetworkPacket {
		bool isPaused;
		bool isComplete;
		float remainingTime;

		TimerPacket() {
			type = Component_Event;
			packetSubType = None;
			size = sizeof(TimerPacket) - sizeof(GamePacket);
		}
	};




	class TimerNetworkComponent : public TimerComponent, public INetworkComponent
	{
	public:
		TimerNetworkComponent(GameObject & gameObject, float timeRemaining, int objId, int ownId, int componId, int pFabId, bool clientOwned) :
			TimerComponent(gameObject, timeRemaining), INetworkComponent(objId, ownId, componId, pFabId, clientOwned) {}

		~TimerNetworkComponent() {
		}

		void Update(float dt) override {
			if (clientOwned) {
				TimerComponent::Update(dt);
				SendTimerPacket();
			}
			else {
				std::cout << "Timer Component Updated" << std::endl;
				std::cout << "Remaining Time: " << remainingTime << std::endl;
			}
		}

		virtual std::unordered_set<std::type_index>& GetDerivedTypes() const override {
			static std::unordered_set<std::type_index> derivedTypes = {
				typeid(TimerComponent),
				typeid(IComponent),
				typeid(INetworkComponent)
			};
			return derivedTypes;
		}

		bool ReadEventPacket(INetworkPacket& p) override {
			if (p.packetSubType == None) {
				TimerPacket* timerPacket = (TimerPacket*)&p;
				isComplete = timerPacket->isComplete;
				remainingTime = timerPacket->remainingTime;
				return true;
			}
			else return false;
		}

		void SendTimerPacket() {
			TimerPacket* timerPacket = new TimerPacket();
			timerPacket->isComplete = isComplete;
			timerPacket->remainingTime = remainingTime;
			SendEventPacket(timerPacket);
			std::cout << "Timer Packet Sent" << std::endl;
		}
	};



}



#endif