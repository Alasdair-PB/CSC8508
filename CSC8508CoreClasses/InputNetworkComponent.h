//
// Contributors: Alasdair
//

#ifndef INPUTNETWORKCOMPONENT_H
#define INPUTNETWORKCOMPONENT_H

#include "INetworkComponent.h"
#include "InputComponent.h"

using std::vector;

namespace NCL::CSC8508
{

	enum InputTypes {
		None,
		Axis,
		Button
	};


	struct InputAxisPacket : INetworkPacket
	{
		uint32_t axisID = -1;
		float axisValue; 

		InputAxisPacket() {
			type = Component_Event;
			packetSubType = Axis;
			size = sizeof(InputAxisPacket) - sizeof(INetworkPacket) - sizeof(GamePacket);
		}
	};

	struct InputButtonPacket : INetworkPacket
	{
		uint32_t buttonID = -1;
		bool held;

		InputButtonPacket() {
			type = Component_Event;
			packetSubType = Button;
			size = sizeof(InputButtonPacket) - sizeof(INetworkPacket) - sizeof(GamePacket);
		}
	};

	class InputNetworkComponent : public INetworkComponent, public InputComponent
	{
	public:
		InputNetworkComponent(GameObject& gameObject, int objId, int ownId, bool clientOwned) :
			InputComponent(gameObject), 
			INetworkComponent(objId, ownId, clientOwned) {}

		~InputNetworkComponent() = default;

		void OnAwake() override {
			for (auto binding : activeController->GetBoundButtons()) {
				lastBoundState[binding] = false;
			}

			boundAxis = activeController->GetBoundAxis();
			for (auto binding : boundAxis) {
				lastAxisState[binding] = false;
			}
		}

		void Update(float deltaTime) override
		{
			if (clientOwned) 
				InputComponent::Update(deltaTime);
		}

		float GetNamedAxis(const std::string& name) override
		{
			if (clientOwned) 
				return activeController->GetNamedAxis(name);
			else {
				auto binding = activeController->GetNamedAxisBinding(name);
				return lastAxisState[binding];
			}
		}

		/*
			InputButtonPacket* buttonPacket = new InputButtonPacket();
			NetworkEvent packet = NetworkEvent(buttonPacket);
			EventManager::Call(buttonPacket);
		*/

	protected:
		vector<uint32_t> boundAxis;
		std::map<uint32_t, bool> lastBoundState;
		std::map<uint32_t, float> lastAxisState;

		 bool ReadFullPacket(INetworkPacket& p) override
		 {

		 }
		 vector<GamePacket*>  WriteFullPacket() override
		 {
			
		 }


		 vector<GamePacket*> WriteDeltaPacket(bool* deltaFrame, int stateID) override
		 {
			 vector<GamePacket*> packets;
			for (auto binding : boundButtons)
			{
				bool buttonDown = activeController->GetButton(binding.first);
				if (buttonDown || lastBoundState[binding.first]);
				{
					InputButtonPacket* buttonPacket = new InputButtonPacket();
					buttonPacket->buttonID = binding.first;
					buttonPacket->held = buttonDown;
					buttonPacket->state = lastFullState.stateID++;

					lastBoundState[binding.first] = buttonDown;
					packets.push_back(buttonPacket);
				}
			}
			for (auto binding : boundAxis)
			{
				float axisValue = activeController->GetAxis(binding);
				if (axisValue != lastAxisState[binding]) {

					InputAxisPacket* axisPacket = new InputAxisPacket();
					axisPacket->axisID = binding;
					axisPacket->axisValue = axisValue;
					axisPacket->state = lastFullState.stateID++;
					lastAxisState[binding] = axisValue;

					packets.push_back(axisPacket);
				}
			}
			return packets;
		}

		bool ReadDeltaPacket(INetworkPacket& p) override
		{

		}

		bool ReadEventPacket(INetworkPacket& p) override
		{
			if (p.packetSubType == InputTypes::Axis) {
				InputAxisPacket p = (InputAxisPacket&)p;

			}
		}
	};
}

#endif //INPUTNETWORKCOMPONENT_H
