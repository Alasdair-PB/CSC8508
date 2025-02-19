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
		TransformNetworkState(): position(0,0,0), orientation(0.0,0.0,0.0,0.0) {}
		~TransformNetworkState() = default;

		Vector3 position;
		Quaternion orientation;
	};

	struct FullPacket : public IFullNetworkPacket {
		TransformNetworkState fullState;

		FullPacket() {
			type = Full_State;
			size = sizeof(FullPacket) - sizeof(GamePacket);
		}
	};

	struct DeltaPacket : public IDeltaNetworkPacket {
		char	pos[3];
		char	orientation[4];

		DeltaPacket() {
			type = Delta_State;
			size = sizeof(DeltaPacket) - sizeof(GamePacket);
		}
	};

	class TransformNetworkComponent :public IComponent, public INetworkDeltaComponent
	{
	public:
		TransformNetworkComponent(GameObject& gameObject, int objId, int ownId, int componId, bool clientOwned): 
			INetworkDeltaComponent(objId, ownId, componId, clientOwned, new TransformNetworkState), 
			IComponent(gameObject),
			myObject(gameObject) {}
		
		~TransformNetworkComponent() = default;

		void OnAwake() override { 
			lastFullState = new TransformNetworkState();
		}
		void Update(float deltaTime) override {}

		virtual std::unordered_set<std::type_index>& GetDerivedTypes() const override {
			static std::unordered_set<std::type_index> types = {
				std::type_index(typeid(IComponent)),				
				std::type_index(typeid(INetworkComponent)),
				std::type_index(typeid(INetworkDeltaComponent))
			};
			return types;
		}

		vector<GamePacket*> WriteDeltaPacket(bool* deltaFrame, int stateID) override 
		{ 
			vector<GamePacket*> packets;

			DeltaPacket* dp = new DeltaPacket();
			TransformNetworkState state;

			if (!GetNetworkState(stateID, &state))
				return packets;

			Vector3 currentPos = GetGameObject().GetTransform().GetPosition();
			Quaternion currentOrientation = GetGameObject().GetTransform().GetOrientation();

			currentPos -= state.position;
			currentOrientation -= state.orientation;

			dp->pos[0] = (char)currentPos.x;
			dp->pos[1] = (char)currentPos.y;
			dp->pos[2] = (char)currentPos.z;

			dp->orientation[0] = (char)(currentOrientation.x * 127.0f);
			dp->orientation[1] = (char)(currentOrientation.y * 127.0f);
			dp->orientation[2] = (char)(currentOrientation.z * 127.0f);
			dp->orientation[3] = (char)(currentOrientation.w * 127.0f);

			SetPacketOwnership(dp);
			packets.push_back(dp);
			return packets;
		}

		vector<GamePacket*> WriteFullPacket() override
		{ 
			vector<GamePacket*> packets;
			FullPacket* fp = new FullPacket();

			fp->fullState.position = myObject.GetTransform().GetPosition();
			fp->fullState.orientation = myObject.GetTransform().GetOrientation();
			fp->fullState.stateID = lastFullState->stateID++;

			SetPacketOwnership(fp);

			packets.push_back(fp);
			return packets;
		}

	protected:
		vector<uint32_t> boundAxis;
		std::map<uint32_t, bool> lastBoundState;
		std::map<uint32_t, float> lastAxisState;
		GameObject& myObject; 

		bool ReadDeltaPacket(IDeltaNetworkPacket& idp) override 
		{
			DeltaPacket p = ((DeltaPacket&)idp);
			TransformNetworkState* lastTransformFullState = static_cast<TransformNetworkState*>(lastFullState);

			Vector3 fullPos = lastTransformFullState->position;
			Quaternion fullOrientation = lastTransformFullState->orientation;

			if (!lastTransformFullState)
				return false;

			fullPos.x += p.pos[0];
			fullPos.y += p.pos[1];
			fullPos.z += p.pos[2];

			fullOrientation.x += ((float)p.orientation[0]) / 127.0f;
			fullOrientation.y += ((float)p.orientation[1]) / 127.0f;
			fullOrientation.z += ((float)p.orientation[2]) / 127.0f;
			fullOrientation.w += ((float)p.orientation[3]) / 127.0f;

			GetGameObject().GetTransform().SetPosition(fullPos);
			myObject.GetTransform().SetOrientation(fullOrientation);
			return true;
		}

		bool ReadFullPacket(IFullNetworkPacket& ifp) override
		{
			FullPacket p = ((FullPacket&) ifp);
			if (p.fullState.stateID < lastFullState->stateID)
				return false;

			std::cout << p.fullState.position.y << std::endl;

			((TransformNetworkState*)lastFullState)->orientation = p.fullState.orientation;
			((TransformNetworkState*)lastFullState)->position = p.fullState.position;
			((TransformNetworkState*)lastFullState)->stateID = p.fullState.stateID;

			TransformNetworkState* lastTransformFullState = static_cast<TransformNetworkState*>(lastFullState);

			if (!lastTransformFullState)
				return false;

			myObject.GetTransform().SetPosition(lastTransformFullState->position);
			myObject.GetTransform().SetOrientation(lastTransformFullState->orientation);

			stateHistory.emplace_back(lastFullState);
			return true;
		}

		bool ReadEventPacket(INetworkPacket& p) override { return true;}
	};
}

#endif //TRANSFORMNETWORKCOMPONENT_H
