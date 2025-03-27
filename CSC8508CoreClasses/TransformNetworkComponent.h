//
// Contributors: Alasdair
//

#ifndef TRANSFORMNETWORKCOMPONENT_H
#define TRANSFORMNETWORKCOMPONENT_H

#include "INetworkComponent.h"
#include "INetworkDeltaComponent.h"
#include "Transform.h"
#include "GameObject.h"

using std::vector;

namespace NCL::CSC8508
{
	class TransformNetworkState : public INetworkState {
	public:
		TransformNetworkState(): position(0,0,0) {}
		~TransformNetworkState() = default;

		Vector3 position;
	};

	struct FullPacket : public IFullNetworkPacket {
		TransformNetworkState fullState;

		FullPacket() {
			type = Full_State;
			size = sizeof(FullPacket) - sizeof(GamePacket);
		}
	};

	struct DeltaPacket : public IDeltaNetworkPacket {
		float pos[3];
		DeltaPacket() {
			type = Delta_State;
			size = sizeof(DeltaPacket) - sizeof(GamePacket);
		}
	};

	class TransformNetworkComponent :public IComponent, public INetworkDeltaComponent
	{
	public:
		TransformNetworkComponent(GameObject& gameObject, int objId, int ownId, int componId, int pfabID, bool clientOwned): 
			INetworkDeltaComponent(objId, ownId, componId, pfabID, clientOwned, new TransformNetworkState),
			IComponent(gameObject),
			myObject(gameObject) {}
		
		~TransformNetworkComponent() = default;

		void Update(float deltaTime) override {}

		const int decPnt = 1;
		virtual std::unordered_set<std::type_index>& GetDerivedTypes() const override {
			static std::unordered_set<std::type_index> types = {
				std::type_index(typeid(IComponent)),
				std::type_index(typeid(INetworkComponent)),
				std::type_index(typeid(INetworkDeltaComponent))
			};
			return types;
		}

		vector<GamePacket*> WriteDeltaPacket(bool* deltaFrame) override 
		{ 
			vector<GamePacket*> packets;
			DeltaPacket* dp = new DeltaPacket();
			TransformNetworkState* state = static_cast<TransformNetworkState*>(lastFullState);
			if (state == nullptr) return packets;

			Vector3 currentPos = GetGameObject().GetTransform().GetPosition();
			currentPos -= state->position;

			dp->pos[0] =(currentPos.x * decPnt);
			dp->pos[1] = (currentPos.y * decPnt);
			dp->pos[2] =(currentPos.z * decPnt);
			dp->fullID = state->stateID;

			SetPacketOwnership(dp);
			packets.push_back(dp);
			return packets;
		}

		vector<GamePacket*> WriteFullPacket() override
		{ 
			vector<GamePacket*> packets;
			FullPacket* fp = new FullPacket();
			TransformNetworkState* state = static_cast<TransformNetworkState*>(lastFullState);

			state->position = myObject.GetTransform().GetPosition();
			state->stateID++;
			fp->fullState.position = myObject.GetTransform().GetPosition();
			fp->fullState.stateID = state->stateID;

			if (clientOwned && state->stateID >= MAX_PACKETID)
				state->stateID = 0;

			SetPacketOwnership(fp);
			packets.push_back(fp);
			return packets;
		}

	protected:
		GameObject& myObject; 

		bool ReadDeltaPacket(IDeltaNetworkPacket& idp) override 
		{
			DeltaPacket p = ((DeltaPacket&)idp);
			if (p.fullID != lastFullState->stateID) return false;

			TransformNetworkState* lastTransformFullState = static_cast<TransformNetworkState*>(lastFullState);		
			if (!lastTransformFullState) return false;

			Vector3 fullPos = lastTransformFullState->position;
			fullPos.x += (p.pos[0]/ decPnt);
			fullPos.y += (p.pos[1]/ decPnt);
			fullPos.z += (p.pos[2]/ decPnt);

			GetGameObject().GetTransform().SetPosition(fullPos);
			return true;
		}
	
		bool ReadFullPacket(IFullNetworkPacket& ifp) override {
			int newStateId = 0; 			
			FullPacket p = ((FullPacket&) ifp);		

			if (!UpdateFullStateHistory<TransformNetworkState, FullPacket>(p, &newStateId))
				return false;
			
			((TransformNetworkState*)lastFullState)->position = p.fullState.position;
			((TransformNetworkState*)lastFullState)->stateID = newStateId;
			TransformNetworkState* lastTransformFullState = static_cast<TransformNetworkState*>(lastFullState);
			
			if (!lastTransformFullState) return false;
			myObject.GetTransform().SetPosition(lastTransformFullState->position);
			return true;
		}
		bool ReadEventPacket(INetworkPacket& p) override { return true;}
	};
}

#endif //TRANSFORMNETWORKCOMPONENT_H
