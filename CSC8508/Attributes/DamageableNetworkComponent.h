#pragma once

#include "IComponent.h"
#include "INetworkDeltaComponent.h"
#include "DamageableComponent.h"

namespace NCL::CSC8508
{
	class DamageableNetworkState : public INetworkState {
	public:
		DamageableNetworkState() : health(0){}
		~DamageableNetworkState() = default;
		int health;
	};

	struct DamFullPacket : public IFullNetworkPacket {
		DamageableNetworkState fullState;

		DamFullPacket() {
			type = Full_State;
			size = sizeof(DamFullPacket) - sizeof(GamePacket);
		}
	};

	class DamageableNetworkComponent : public DamageableComponent, public INetworkDeltaComponent {
	public:

		DamageableNetworkComponent(GameObject& gameObject, int initialHealth, int initialMaxHealth, int objId, int ownId, int componId, int pfab, bool clientOwned) :
			INetworkDeltaComponent(objId, ownId, componId, pfab, clientOwned, new DamageableNetworkState),
			DamageableComponent(gameObject, initialHealth, initialMaxHealth) {
		}

		~DamageableNetworkComponent() = default;

		const int decPnt = 1;
		virtual std::unordered_set<std::type_index>& GetDerivedTypes() const override {
			static std::unordered_set<std::type_index> types = {
				std::type_index(typeid(IComponent)),
				std::type_index(typeid(INetworkComponent)),
				std::type_index(typeid(DamageableComponent)),
				std::type_index(typeid(INetworkDeltaComponent))
			};
			return types;
		}

		void InitHealthUI() override {
			if (!IsOwner()) return;
			DamageableComponent::InitHealthUI();
		}

		void OnAwake() override{ InitHealthUI(); }

		void Update(float deltaTime) override {
			if (!IsOwner()) return;
			DamageableComponent::Update(deltaTime);
		}

		vector<GamePacket*> WriteDeltaPacket(bool* deltaFrame) override { return WriteFullPacket();}

		vector<GamePacket*> WriteFullPacket() override
		{
			vector<GamePacket*> packets;
			DamFullPacket* fp = new DamFullPacket();
			DamageableNetworkState* state = static_cast<DamageableNetworkState*>(lastFullState);

			state->health = health;

			state->stateID++;
			fp->fullState.health = health;
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
			DamFullPacket p = ((DamFullPacket&)ifp);

			if (!UpdateFullStateHistory<DamageableNetworkState, DamFullPacket>(p, &newStateId))
				return false;

			((DamageableNetworkState*)lastFullState)->health = p.fullState.health;
			((DamageableNetworkState*)lastFullState)->stateID = newStateId;
			DamageableNetworkState* lastTransformFullState = static_cast<DamageableNetworkState*>(lastFullState);

			if (!lastTransformFullState) return false;
			health = lastTransformFullState->health;
			return true;
		}
		bool ReadEventPacket(INetworkPacket& p) override { return true; }

	protected:

	};
}