//
// Contributors: Alasdair
//

#ifndef FULLTRANSFORMNETWORKCOMPONENT_H
#define FULLTRANSFORMNETWORKCOMPONENT_H

#include "INetworkComponent.h"
#include "INetworkDeltaComponent.h"
#include "Transform.h"
#include "GameObject.h"

using std::vector;

namespace NCL::CSC8508
{
	class FullTransformNetworkState : public INetworkState {
	public:
		FullTransformNetworkState(): position(0,0,0) /*, orientation(0.0, 0.0, 0.0, 0.0)*/ {}
		~FullTransformNetworkState() = default;

		Vector3 position;
		Quaternion orientation;
	};

	struct FullPacket : public IFullNetworkPacket {
		FullTransformNetworkState fullState;

		FullPacket() {
			type = Full_State;
			size = sizeof(FullPacket) - sizeof(GamePacket);
		}
	};

	struct DeltaPacket : public IDeltaNetworkPacket {
		float pos[3];
		char orientation[4];

		DeltaPacket() {
			type = Delta_State;
			size = sizeof(DeltaPacket) - sizeof(GamePacket);
		}
	};

	class FullTransformNetworkComponent :public IComponent, public INetworkDeltaComponent
	{
	public:
		FullTransformNetworkComponent(GameObject& gameObject, int objId, int ownId, int componId, bool clientOwned):
			INetworkDeltaComponent(objId, ownId, componId, clientOwned, new FullTransformNetworkState), 
			IComponent(gameObject),
			myObject(gameObject) {}
		
		~FullTransformNetworkComponent() = default;

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
			FullTransformNetworkState* state = static_cast<FullTransformNetworkState*>(lastFullState);
			if (state == nullptr) return packets;

			Vector3 currentPos = GetGameObject().GetTransform().GetPosition();
			Quaternion currentOrientation = GetGameObject().GetTransform().GetOrientation();

			currentPos -= state->position;
			currentOrientation -= state->orientation;

			dp->pos[0] =(currentPos.x * decPnt);
			dp->pos[1] = (currentPos.y * decPnt);
			dp->pos[2] =(currentPos.z * decPnt);

			dp->orientation[0] =(currentOrientation.x * 12700.0f);
			dp->orientation[1] = (currentOrientation.y * 12700.0f);
			dp->orientation[2] =(currentOrientation.z * 12700.0f);
			dp->orientation[3] = (currentOrientation.w * 12700.0f);
			dp->fullID = state->stateID;

			SetPacketOwnership(dp);
			packets.push_back(dp);
			return packets;
		}

		vector<GamePacket*> WriteFullPacket() override
		{ 
			vector<GamePacket*> packets;
			FullPacket* fp = new FullPacket();
			FullTransformNetworkState* state = static_cast<FullTransformNetworkState*>(lastFullState);

			state->position = myObject.GetTransform().GetPosition();
			state->orientation = myObject.GetTransform().GetOrientation();

			state->stateID++;
			fp->fullState.position = myObject.GetTransform().GetPosition();
			fp->fullState.orientation = myObject.GetTransform().GetOrientation();
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

			FullTransformNetworkState* lastTransformFullState = static_cast<FullTransformNetworkState*>(lastFullState);
			if (!lastTransformFullState) return false;

			Vector3 fullPos = lastTransformFullState->position;
			Quaternion fullOrientation = lastTransformFullState->orientation;

			fullPos.x += (p.pos[0]/ decPnt);
			fullPos.y += (p.pos[1]/ decPnt);
			fullPos.z += (p.pos[2]/ decPnt);

			fullOrientation.x += ((float)p.orientation[0]) / 12700.0f;
			fullOrientation.y += ((float)p.orientation[1]) / 12700.0f;
			fullOrientation.z += ((float)p.orientation[2]) / 12700.0f;
			fullOrientation.w += ((float)p.orientation[3]) / 12700.0f;

			GetGameObject().GetTransform().SetPosition(fullPos);
			myObject.GetTransform().SetOrientation(fullOrientation);
			return true;
		}
	
		bool ReadFullPacket(IFullNetworkPacket& ifp) override {
			int newStateId = 0; 			
			FullPacket p = ((FullPacket&) ifp);		

			if (!UpdateFullStateHistory<FullTransformNetworkState, FullPacket>(p, &newStateId))
				return false;
			
			((FullTransformNetworkState*)lastFullState)->orientation = p.fullState.orientation;
			((FullTransformNetworkState*)lastFullState)->position = p.fullState.position;
			((FullTransformNetworkState*)lastFullState)->stateID = newStateId;
			FullTransformNetworkState* lastTransformFullState = static_cast<FullTransformNetworkState*>(lastFullState);
			
			if (!lastTransformFullState) return false;
			myObject.GetTransform().SetPosition(lastTransformFullState->position);
			myObject.GetTransform().SetOrientation(lastTransformFullState->orientation);
			return true;
		}
		bool ReadEventPacket(INetworkPacket& p) override { return true;}
	};
}

#endif //FULLTRANSFORMNETWORKCOMPONENT_H
