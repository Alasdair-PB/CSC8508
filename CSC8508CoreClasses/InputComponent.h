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
	class InputComponent : public IComponent
	{
	public:
		InputComponent(GameObject& gameObject, Controller* controller) : IComponent(gameObject), activeController(controller) {}
		~InputComponent() = default;

		virtual void Update(float deltaTime) override
		{
			for (auto binding : boundButtons) {
				if (activeController->GetButton(binding.first));
					EventManager::Call(binding.second);
			}
		}

		virtual float GetNamedAxis(const std::string& name) {
			return activeController->GetNamedAxis(name);
		}

	protected:
		std::map<uint32_t, Event*> boundButtons;
		const Controller* activeController = nullptr;
	};
}

#endif //INPUTCOMPONENT_H
