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
		float deltaTime = 0;
		float mouseYaw = 0;
		int historyStamp = 0;

		InputDeltaPacket() {
			type = Component_Event;
			packetSubType = Delta;
			historyStamp = 0;
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
			INetworkComponent(objId, ownId, componId, clientOwned), reset(true) {}

		~InputNetworkComponent() = default;

		void OnAwake() override {	
			InputComponent::OnAwake();
			boundAxis = activeController->GetBoundAxis();
			for (uint32_t binding : boundButtons) lastBoundState[binding] = false;
			for (uint32_t binding : boundAxis) lastAxisState[binding] = 0;
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
			if (clientOwned) {
				UpdateMouseGameWorldPitchYaw();
				UpdateDeltaAxis(deltaTime);
			}
			else {
				reset = true;
				while (!historyQueue.empty()) {
					HistoryData data = historyQueue.front();
					if (data.historyStamp >= lastHistoryEntry) {
						lastHistoryEntry = data.historyStamp;
						lastAxisState = data.axisMap;
						mouseGameWorldYaw = data.mouseGameWorldYaw;
						reset = false;
					}
					historyQueue.pop();
				}
			}
		}

		float GetNamedAxis(const std::string& name) override
		{
			if (clientOwned) return activeController->GetNamedAxis(name);
			else {
				uint32_t binding = activeController->GetNamedAxisBinding(name);
				return reset ? 0 : lastAxisState[binding];
			}
		}

	private:
		bool reset;
	protected:

		struct HistoryData
		{
			float deltaTime;
			float mouseGameWorldYaw;
			int historyStamp;
			std::map<uint32_t, float> axisMap;

			HistoryData(std::map<uint32_t, float> axisMap, float mouseGameWorldYaw, float deltaTime, int historyStamp) {
				this->axisMap = axisMap;
				this->deltaTime = deltaTime;
				this->mouseGameWorldYaw = mouseGameWorldYaw; 
				this->historyStamp = historyStamp;
			}
		};

		vector<uint32_t> boundAxis;
		std::map<uint32_t, float> lastAxisState;
		std::map<uint32_t, bool> lastBoundState;
		std::queue<HistoryData> historyQueue;
		int lastHistoryEntry = 0;
			 
		bool ReadAxisPacket(InputAxisPacket pck) {
			lastAxisState[pck.axisID] = pck.axisValue;
			return true;
		}

		bool ReadDeltaPacket(InputDeltaPacket pck) 
		{
			std::map<uint32_t, float> elements = std::map<uint32_t, float>();
			for (int i = 0; i < MAX_AXIS_COUNT; i++)
				elements[pck.axisIDs[i]] = pck.axisValues[i];
			HistoryData data = HistoryData(elements, pck.mouseYaw, pck.deltaTime, pck.historyStamp);
			historyQueue.push(data);
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

		void UpdateDeltaAxis(float deltaTime) {
			int t = boundAxis.size();				
			InputDeltaPacket* deltaPacket = new InputDeltaPacket();
			bool hasChanged = false;
			for (int i = 0; i < t; i += MAX_AXIS_COUNT){
				for (int j = 0; j < MAX_AXIS_COUNT && (i + j) < t; j++) {
					uint32_t binding = boundAxis[i + j];
					float axisValue = activeController->GetAxis(binding);

					if (axisValue != 0) 
						hasChanged = true;

					deltaPacket->axisIDs[j] = binding;
					deltaPacket->axisValues[j] = axisValue;
					lastAxisState[binding] = axisValue;
				}
			}				
			
			deltaPacket->deltaTime = deltaTime;
			deltaPacket->mouseYaw = mouseGameWorldYaw;
			deltaPacket->historyStamp = lastHistoryEntry;

			if (hasChanged) {				
				lastHistoryEntry++;
				SendEventPacket(deltaPacket);
			}
			delete deltaPacket;
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
