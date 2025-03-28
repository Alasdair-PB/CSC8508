#include "Inspector.h"
#include "IComponent.h"
#include <iostream>
#include <tuple>
#include <string>

Inspector::Inspector() : positionInfo(new Vector3()), 
	scaleInfo(new Vector3()), orientationInfo(new Vector4()), isEnabled(new bool()), saveDestination(new std::string("Deafult.pfab"))
{
	inspectorBar = new UIElementsGroup(
		ImVec2(0.6f, 0.3f), 
		ImVec2(0.3f, 0.5f),
		1.0f, 
		"Inspector",
		0.0f, 
		ImGuiWindowFlags_NoResize);

	inspectorBar->PushToggle("GameObject:", isEnabled, 0.05f);
	inspectorBar->PushStatelessInputFieldElement("GameObject:", saveDestination);
	inspectorBar->PushStatelessButtonElement(ImVec2(0.05f, 0.05f), "Save Pfab", 
		[this]() {
			std::cout << "Button down" << std::endl;
			if (focus) {
				focus->Save(*saveDestination);
				std::cout << "saved?" << std::endl;

			}
		});
	inspectorBar->PushVectorElement(positionInfo, 0.05f);
	inspectorBar->PushVectorElement(scaleInfo, 0.05f);
	inspectorBar->PushQuaternionElement(orientationInfo, 0.05f);
}

Inspector::~Inspector() { 
	delete inspectorBar; 
	delete positionInfo; 
	delete scaleInfo;
	delete orientationInfo;
}

void Inspector::RenderIComponents() {
	if (!focus) return;
	inspectorBar->RemoveElementsFromIndex(6);

	for (IComponent* component : focus->GetAllComponents()) {
		component->PushIComponentElementsInspector(*inspectorBar, 0.05f);
	}
}
