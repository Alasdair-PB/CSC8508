//
// Contributors: Alasdair
//

#ifndef INPUTNETWORKCOMPONENT_H
#define INPUTNETWORKCOMPONENT_H

#include "INetworkComponent.h"
#include "INetworkDeltaComponent.h"
#include "InputComponent.h"
#include <queue>

using std::vector;

namespace NCL::CSC8508
{
	enum InputTypes {
		None,
		Axis,
		Button,
		Delta,
		Bound
	};

	struct InputAxisPacket : INetworkPacket
	{
		uint32_t axisID = -1;
		float axisValue = 0; 

		InputAxisPacket() {
			type = Component_Event;
			packetSubType = Axis;
			size = sizeof(InputAxisPacket) - sizeof(GamePacket);
		}
	};

	const int MAX_AXIS_COUNT = 5;
	struct InputDeltaPacket : INetworkPacket {
		uint32_t axisIDs[MAX_AXIS_COUNT];
		float axisValues[MAX_AXIS_COUNT];

		InputDeltaPacket() {
			type = Component_Event;
			packetSubType = Delta;
			size = sizeof(InputDeltaPacket) - sizeof(GamePacket);
		}
	};

	struct InputButtonPacket : INetworkPacket
	{
		uint32_t buttonID = -1;
		bool held = false;

		InputButtonPacket() {
			type = Component_Event;
			packetSubType = Button;
			size = sizeof(InputButtonPacket) - sizeof(GamePacket);
		}
	};

	class InputNetworkComponent :public InputComponent, public INetworkComponent
	{
	public:
		InputNetworkComponent(GameObject& gameObject, Controller* controller, int objId, int ownId, int componId, bool clientOwned) :
			InputComponent(gameObject, controller), 
			INetworkComponent(objId, ownId, componId, clientOwned)
		{}

		~InputNetworkComponent() = default;

		void OnAwake() override {	
			InputComponent::OnAwake();
			boundAxis = activeController->GetBoundAxis();
			for (uint32_t binding : boundButtons) lastBoundState[binding] = false;
			for (uint32_t binding : boundAxis) lastAxisState[binding] = false;
		}

		virtual std::unordered_set<std::type_index>& GetDerivedTypes() const override {
			static std::unordered_set<std::type_index> types = {
				std::type_index(typeid(IComponent)),				
				std::type_index(typeid(InputComponent)),				
				std::type_index(typeid(INetworkComponent))
			};
			return types;
		}

		void EarlyUpdate(float deltaTime) override
		{
			if (clientOwned) UpdateDeltaAxis();
			else {
				if (!historyQueue.empty()) {
					lastAxisState = historyQueue.front();
					historyQueue.pop();
				}
				else {
					for (auto state : lastAxisState)
						state.second = 0;
				}
			}
		}

		float GetNamedAxis(const std::string& name) override
		{
			if (clientOwned) return activeController->GetNamedAxis(name);
			else {
				uint32_t binding = activeController->GetNamedAxisBinding(name);
				return lastAxisState[binding];
			}
		}

	protected:
		bool clearLastFrame = false;
		vector<uint32_t> boundAxis;
		std::map<uint32_t, float> lastAxisState;

		std::map<uint32_t, bool> lastBoundState;
		std::queue<std::map<uint32_t, float>> historyQueue;

		bool ReadAxisPacket(InputAxisPacket pck) {
			lastAxisState[pck.axisID] = pck.axisValue;
			return true;
		}

		bool ReadDeltaPacket(InputDeltaPacket pck) 
		{
			std::map<uint32_t, float> elements = std::map<uint32_t, float>();
			for (int i = 0; i < MAX_AXIS_COUNT; i++)
				elements[pck.axisIDs[i]] = pck.axisValues[i];
			historyQueue.push(elements);
			return true;
		}

		bool ReadButtonPacket(InputButtonPacket pck) {
			if (pck.held) CallInputEvent(boundButtons[pck.buttonID]);
			// May add events for on release
			lastBoundState[pck.buttonID] = pck.held;
			return true;
		}

		bool ReadEventPacket(INetworkPacket& p) override
		{
			if (p.packetSubType == InputTypes::Axis)
				return ReadAxisPacket((InputAxisPacket&)p);
			else if (p.packetSubType == InputTypes::Delta) 
				return ReadDeltaPacket((InputDeltaPacket&)p);
			else if (p.packetSubType == InputTypes::Button) 
				return ReadButtonPacket((InputButtonPacket&)p);
			return false;
		}	
		
		void SendAxisPacket(uint32_t binding, float axisValue) {
			InputAxisPacket* axisPacket = new InputAxisPacket();
			axisPacket->axisID = binding;
			axisPacket->axisValue = axisValue;

			SendEventPacket(axisPacket);
			lastAxisState[binding] = axisValue;
			delete axisPacket;
		}

		void UpdateDeltaAxis() {
			int t = boundAxis.size();
			for (int i = 0; i < t; i += MAX_AXIS_COUNT) {
				InputDeltaPacket* deltaPacket = new InputDeltaPacket();
				for (int j = 0; j < MAX_AXIS_COUNT && (i + j) < t; j++) 
				{
					uint32_t binding = boundAxis[i + j];
					float axisValue = activeController->GetAxis(binding);
					deltaPacket->axisIDs[j] = binding;
					deltaPacket->axisValues[j] = axisValue; // : axisValue - lastAxisState[binding];
					lastAxisState[binding] = axisValue;
				}
				SendEventPacket(deltaPacket);
				delete deltaPacket;
			}
		}

		void UpdateBoundButtons() 
		{
			for (auto binding : boundButtons)
			{
				bool buttonDown = activeController->GetButton(binding);
				if (buttonDown || lastBoundState[binding]);
				{
					InputButtonPacket* buttonPacket = new InputButtonPacket();
					buttonPacket->buttonID = binding;
					buttonPacket->held = buttonDown;

					SendEventPacket(buttonPacket);
					CallInputEvent(binding);
					lastBoundState[binding] = buttonDown;
					delete buttonPacket;
				}
			}
		}
	};
}

#endif //INPUTNETWORKCOMPONENT_H
