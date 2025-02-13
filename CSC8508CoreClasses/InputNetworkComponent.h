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
			size = sizeof(InputAxisPacket) - sizeof(GamePacket);
		}
	};

	struct InputButtonPacket : INetworkPacket
	{
		uint32_t buttonID = -1;
		bool held;

		InputButtonPacket() {
			type = Component_Event;
			packetSubType = Button;
			size = sizeof(InputButtonPacket) - sizeof(GamePacket);
		}
	};

	class InputNetworkComponent : public INetworkComponent, public InputComponent
	{
	public:
		InputNetworkComponent(GameObject& gameObject, Controller* controller, int objId, int ownId, int componId, bool clientOwned) :
			InputComponent(gameObject, controller), 
			INetworkComponent(objId, ownId, componId, clientOwned) {}

		~InputNetworkComponent() = default;

		void OnAwake() override {
			for (uint32_t binding : activeController->GetBoundButtons()) {
				lastBoundState[binding] = false;
			}

			boundAxis = activeController->GetBoundAxis();
			for (uint32_t binding : boundAxis) {
				lastAxisState[binding] = false;
			}
		}

		virtual std::unordered_set<std::type_index>& GetDerivedTypes() const override {
			static std::unordered_set<std::type_index> types = { 
				std::type_index(typeid(IComponent)),
				std::type_index(typeid(InputComponent)),
				std::type_index(typeid(INetworkComponent))
			};
			return types;
		}

		void Update(float deltaTime) override
		{
			if (clientOwned) 
			{
				for (auto binding : boundButtons)
				{
					bool buttonDown = activeController->GetButton(binding.first);
					if (buttonDown || lastBoundState[binding.first]);
					{
						InputButtonPacket* buttonPacket = new InputButtonPacket();
						buttonPacket->buttonID = binding.first;
						buttonPacket->held = buttonDown;

						SendEventPacket(buttonPacket);
						EventManager::Call(binding.second);

						lastBoundState[binding.first] = buttonDown;
						delete buttonPacket;
					}
				}

				for (auto binding : boundAxis)
				{
					float axisValue = activeController->GetAxis(binding);
					if (axisValue != lastAxisState[binding]) {

						InputAxisPacket* axisPacket = new InputAxisPacket();
						axisPacket->axisID = binding;
						axisPacket->axisValue = axisValue;
						
						SendEventPacket(axisPacket);
						lastAxisState[binding] = axisValue;
						delete axisPacket;

					}
				}
			}
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

	protected:
		vector<uint32_t> boundAxis;
		std::map<uint32_t, bool> lastBoundState;
		std::map<uint32_t, float> lastAxisState;

		bool ReadEventPacket(INetworkPacket& p) override
		{
			if (p.packetSubType == InputTypes::Axis) {
				InputAxisPacket pck = (InputAxisPacket&)p;
				lastAxisState[pck.axisID] = pck.axisValue;
			}
			else if (p.packetSubType == InputTypes::Button) {
				InputButtonPacket pck = (InputButtonPacket&)p;

				if (pck.held)
					EventManager::Call(boundButtons[pck.buttonID]);
				// May add events for on release
				lastBoundState[pck.buttonID] = pck.held;
			}

			return true;
		}
	};
}

#endif //INPUTNETWORKCOMPONENT_H
