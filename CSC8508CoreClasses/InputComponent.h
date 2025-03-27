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
			InputButtonEvent(uint32_t buttonId, GameObject& owner) : owner(owner) {
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
		InputComponent(GameObject& gameObject, Controller* controller) : IComponent(gameObject), activeController(controller), mouseGameWorldYaw(0){}
		~InputComponent() = default;

		virtual void OnAwake() override {
			boundButtons = activeController->GetBoundButtons();
		}

		virtual void EarlyUpdate(float deltaTime) override
		{
			CheckButtonBindings();
			UpdateMouseGameWorldPitchYaw();
		}

		virtual float GetNamedAxis(const std::string& name) {
			return activeController->GetNamedAxis(name);
		}

		Matrix3 GetMouseGameWorldYawMatrix() {
			return Matrix::RotationMatrix3x3(mouseGameWorldYaw, Vector3(0, 1, 0));
		}
		float GetMouseGameWorldYaw() { return mouseGameWorldYaw;}
		float GetMouseGameWorldPitch() { return mouseGameWorldPitch;}

	protected:
		void CallInputEvent(uint32_t button) {
			InputButtonEvent inputEvent = InputButtonEvent(button, GetGameObject());
			EventManager::Call<InputButtonEvent>(&inputEvent);
		}

		void CheckButtonBindings() {
			for (auto binding : boundButtons) {
				if (activeController->GetBoundButton(binding)) {
					uint32_t hashBinding = activeController->GetButtonHashId(binding);
					CallInputEvent(hashBinding);
				}
			}
		}

		void UpdateMouseGameWorldPitchYaw() {

			mouseGameWorldPitch -= GetNamedAxis("YLook");
			mouseGameWorldYaw -= GetNamedAxis("XLook");

			mouseGameWorldPitch = std::min(mouseGameWorldPitch, 90.0f);
			mouseGameWorldPitch = std::max(mouseGameWorldPitch, -90.0f);

			if (mouseGameWorldYaw < 0)
				mouseGameWorldYaw += 360.0f;
			if (mouseGameWorldYaw > 360.0f)
				mouseGameWorldYaw -= 360.0f;
		}

		vector<uint32_t> boundButtons;
		float mouseGameWorldYaw = 0;
		float mouseGameWorldPitch = 0;
		const Controller* activeController = nullptr;
	};
}

#endif //INPUTCOMPONENT_H
