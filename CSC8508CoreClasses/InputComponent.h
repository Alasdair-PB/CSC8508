//
// Contributors: Alasdair
//

#ifndef INPUTCOMPONENT_H
#define INPUTCOMPONENT_H

#include "IComponent.h"
#include "Event.h"
#include "EventManager.h"
#include "../NCLCoreClasses/Controller.h"


using std::vector;

namespace NCL::CSC8508
{		
	class InputButtonEvent : Event {
		public:
			InputButtonEvent(uint32_t eventData, GameObject& owner) : owner(owner) {
				this->buttonId = buttonId;
			}
			uint32_t buttonId;
			GameObject& owner;
		protected:
			uint32_t GetButtonId() const { return this->buttonId; }
			GameObject& GetOwner() const { return this->owner; }
	};

	class InputComponent : public IComponent
	{
	public:
		InputComponent(GameObject& gameObject, Controller* controller) : IComponent(gameObject), activeController(controller) {}
		~InputComponent() = default;

		virtual void OnAwake() override {
			boundButtons = activeController->GetBoundButtons();
		}

		virtual void Update(float deltaTime) override
		{
			for (auto binding : boundButtons) {
				if (activeController->GetButton(binding));
					CallInputEvent(binding);
			}
		}

		virtual float GetNamedAxis(const std::string& name) {
			return activeController->GetNamedAxis(name);
		}

	protected:
		void CallInputEvent(uint32_t button) {
			InputButtonEvent inputEvent = InputButtonEvent(button, GetGameObject());
			EventManager::Call<InputButtonEvent>(&inputEvent);
		}
		vector<uint32_t> boundButtons;
		const Controller* activeController = nullptr;
	};
}

#endif //INPUTCOMPONENT_H
