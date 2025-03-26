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

	const int MAX_AXIS_COUNT = 5;
	const int MAX_BUTTON_COUNT = 5;

	struct InputDeltaPacket : INetworkPacket {
		uint32_t axisIDs[MAX_AXIS_COUNT];
		uint32_t buttonIDs[MAX_BUTTON_COUNT];
		float axisValues[MAX_AXIS_COUNT];
		float deltaTime = 0;
		float mouseYaw = 0;
		float mousePitch = 0;

		int historyStamp = 0;

		InputDeltaPacket() {
			type = Component_Event;
			packetSubType = Delta;
			historyStamp = 0;
			size = sizeof(InputDeltaPacket) - sizeof(GamePacket);
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

		/// <summary>
		/// Manages the tracking of skippedFrames (frames where multiple packets have arrived simultaneously)
		/// Keeps previous frames in history for each skipped frame counted where an input has not arrived. 
		/// </summary>
		void ManageFramePrediction() {
			if (reset) {
				if (skippedFrames > skipLimit) skippedFrames /= 2;
				if (frameCount < skippedFrames) {
					frameCount++;
					skippedFrames--;
					reset = false;
				}
			}
			else 
				frameCount = 0;
		}

		/// <summary>
		/// Set lastAxisState and mouseGameWorldYaw using history data recieved over the network
		/// In cases where multiple packets arrive simultaneously only set a single frames worth of data is set
		/// when this happens skippedFrames is incremented to notify ManageFramePrediction() of skippedFrames/ lostFrames
		/// </summary>
		void PredictUpdate() {
			reset = true;
			if (!historyQueue.empty()) {
				skippedFrames--;
				while (!historyQueue.empty()) {
					HistoryData data = historyQueue.front();

					while (!data.buttonMap.empty()) {
						uint32_t id = data.buttonMap.top();
						lastBoundState[id] = true;
						data.buttonMap.pop();
					}
					
					if (data.historyStamp >= lastHistoryEntry) {
						lastHistoryEntry = data.historyStamp;
						lastAxisState = data.axisMap;
						mouseGameWorldYaw = data.mouseGameWorldYaw;
						mouseGameWorldPitch = data.mouseGameWorldYaw;
						reset = false;
					}
					skippedFrames++;
					historyQueue.pop();
				}
			}
		}

		void PredictButtonInput() {
			for (const auto& [key, value] : lastBoundState) {
				if (lastBoundState[key]) {
					CallInputEvent(key);
					lastBoundState[key] = false;
				}
			}
		}

		void EarlyUpdate(float deltaTime) override
		{
			if (clientOwned) {				
				CheckButtonBindings();
				UpdateMouseGameWorldPitchYaw();
				UpdateDeltaAxis(deltaTime);
			}
			else {
				PredictUpdate();
				PredictButtonInput();
				ManageFramePrediction();
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
		int frameCount;
		int skippedFrames;
		const int skipLimit = 10;
	protected:

		struct HistoryData
		{
			float deltaTime;
			float mouseGameWorldYaw;
			float mouseGameWorldPitch;
			int historyStamp;
			std::map<uint32_t, float> axisMap;
			std::stack<uint32_t> buttonMap;

			HistoryData(std::map<uint32_t, float> axisMap, std::stack<uint32_t> buttonMap, float mouseGameWorldYaw, float mouseGameWorldPitch, float deltaTime, int historyStamp) {
				this->axisMap = axisMap;
				this->buttonMap = buttonMap;
				this->deltaTime = deltaTime;
				this->mouseGameWorldYaw = mouseGameWorldYaw; 
				this->mouseGameWorldPitch = mouseGameWorldPitch;
				this->historyStamp = historyStamp;
			}
		};

		vector<uint32_t> boundAxis;
		std::map<uint32_t, float> lastAxisState;
		std::map<uint32_t, bool> lastBoundState;
		std::queue<HistoryData> historyQueue;
		int lastHistoryEntry = 0;
		
		bool ReadDeltaPacket(InputDeltaPacket pck) 
		{
			std::map<uint32_t, float> elements = std::map<uint32_t, float>();
			std::stack<uint32_t> buttonElements = std::stack<uint32_t>();

			for (int i = 0; i < MAX_AXIS_COUNT; i++)
				elements[pck.axisIDs[i]] = pck.axisValues[i];
			for (int i = 0; i < MAX_BUTTON_COUNT; i++)
				if (pck.buttonIDs[i] > 0) buttonElements.push(pck.buttonIDs[i]);
			HistoryData data = HistoryData(elements, buttonElements, pck.mouseYaw, pck.mousePitch, pck.deltaTime, pck.historyStamp);
			historyQueue.push(data);
			return true;
		}

		bool ReadEventPacket(INetworkPacket& p) override
		{
			if (p.packetSubType == InputTypes::Delta) 
				return ReadDeltaPacket((InputDeltaPacket&)p);
			return false;
		}

		void ReadBoundAxis(bool& hasChanged, InputDeltaPacket* deltaPacket) {
			int t = boundAxis.size();
			for (int i = 0; i < t; i += MAX_AXIS_COUNT) {
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
		}

		void ReadBoundButtons(bool& hasChanged, InputDeltaPacket* deltaPacket) {			
			int tB = boundButtons.size();
			for (int i = 0; i < tB; i += MAX_BUTTON_COUNT) {
				for (int j = 0; j < MAX_BUTTON_COUNT && (i + j) < tB; j++) {
					uint32_t binding = boundButtons[i + j];
					bool pressed = activeController->GetBoundButton(binding);
					if (pressed) {
						hasChanged = true;
						deltaPacket->buttonIDs[j] = binding;
					}
				}
			}
		}

		void UpdateDeltaAxis(float deltaTime) {
			InputDeltaPacket* deltaPacket = new InputDeltaPacket();
			bool hasChanged = false;

			ReadBoundAxis(hasChanged, deltaPacket);
			ReadBoundButtons(hasChanged, deltaPacket);

			deltaPacket->deltaTime = deltaTime;
			deltaPacket->mouseYaw = mouseGameWorldYaw;
			deltaPacket->mousePitch = mouseGameWorldPitch;
			deltaPacket->historyStamp = lastHistoryEntry;

			if (hasChanged) {				
				lastHistoryEntry++;
				SendEventPacket(deltaPacket);
			}
			delete deltaPacket;
		}
	};
}

#endif //INPUTNETWORKCOMPONENT_H
