#pragma once

#include "IComponent.h"
#include "INetworkDeltaComponent.h"
#include "../CSC8508/StaminaComponent.h"

namespace NCL::CSC8508
{
	class StaminaNetworkState : public INetworkState {
	public:
		StaminaNetworkState() : stamina(0){}
		~StaminaNetworkState() = default;
		int stamina;
	};

	struct StaminaFullPacket : public IFullNetworkPacket {
		StaminaNetworkState fullState;

		StaminaFullPacket() {
			type = Full_State;
			size = sizeof(StaminaFullPacket) - sizeof(GamePacket);
		}
	};

	class StaminaNetworkComponent : public StaminaComponent, public INetworkDeltaComponent {
	public:

		StaminaNetworkComponent(GameObject& gameObject, int initialStamina, int initialMaxStamina, int sRegen, int objId, int ownId, int componId, int pfab, bool clientOwned) :
			INetworkDeltaComponent(objId, ownId, componId, pfab, clientOwned, new StaminaNetworkState),
			StaminaComponent(gameObject, initialStamina, initialMaxStamina, sRegen) {
		}

		~StaminaNetworkComponent() = default;

		const int decPnt = 1;
		virtual std::unordered_set<std::type_index>& GetDerivedTypes() const override {
			static std::unordered_set<std::type_index> types = {
				std::type_index(typeid(IComponent)),
				std::type_index(typeid(INetworkComponent)),
				std::type_index(typeid(StaminaComponent)),
				std::type_index(typeid(INetworkDeltaComponent))
			};
			return types;
		}

		void InitStaminaUI() override {
			if (!IsOwner()) return;
			StaminaComponent::InitStaminaUI();
		}

		void UpdateStaminaUI() override {
			if (!IsOwner()) return;
			StaminaComponent::UpdateStaminaUI();
		}

		void OnAwake() override
		{
			InitStaminaUI();
		}

		vector<GamePacket*> WriteDeltaPacket(bool* deltaFrame) override { return WriteFullPacket();}

		vector<GamePacket*> WriteFullPacket() override
		{
			vector<GamePacket*> packets;
			StaminaFullPacket* fp = new StaminaFullPacket();
			StaminaNetworkState* state = static_cast<StaminaNetworkState*>(lastFullState);

			state->stamina = stamina;

			state->stateID++;
			fp->fullState.stamina = stamina;
			fp->fullState.stateID = state->stateID;

			if (clientOwned && state->stateID >= MAX_PACKETID)
				state->stateID = 0;

			SetPacketOwnership(fp);
			packets.push_back(fp);
			return packets;
		}

		bool ReadDeltaPacket(IDeltaNetworkPacket& idp) override {return true;}

		bool ReadFullPacket(IFullNetworkPacket& ifp) override {
			int newStateId = 0;
			StaminaFullPacket p = ((StaminaFullPacket&)ifp);

			if (!UpdateFullStateHistory<StaminaNetworkState, StaminaFullPacket>(p, &newStateId))
				return false;

			((StaminaNetworkState*)lastFullState)->stamina = p.fullState.stamina;
			((StaminaNetworkState*)lastFullState)->stateID = newStateId;
			StaminaNetworkState* lastTransformFullState = static_cast<StaminaNetworkState*>(lastFullState);

			if (!lastTransformFullState) return false;
			stamina = lastTransformFullState->stamina;
			return true;
		}
		bool ReadEventPacket(INetworkPacket& p) override { return true; }

	protected:

	};
}