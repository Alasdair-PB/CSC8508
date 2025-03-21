#include "Inspector.h"
#include "IComponent.h"
#include <iostream>
#include <tuple>
#include <string>

Inspector::Inspector(){
	inspectorBar = new UIElementsGroup(
		ImVec2(0.6f, 0.3f), 
		ImVec2(0.3f, 0.15f),
		1.0f, 
		"Inspector",
		0.0f, 
		ImGuiWindowFlags_NoResize);

	transformInfo = new Vector3(0, 0, 0);

	std::function<CSC8508::PushdownState::PushdownResult()> funcC =
		[this]()-> CSC8508::PushdownState::PushdownResult {
			return CSC8508::PushdownState::PushdownResult::NoChange;
		};

	inspectorBar->PushButtonElement(ImVec2(0.2f, 0.05f), "Save Pfab", funcC);
	inspectorBar->PushVectorElement(transformInfo, 0.05f);

	// Component Names
	//inspectorBar->PushTextElement("");
}

Inspector::~Inspector() { delete inspectorBar; delete transformInfo; }

void Inspector::RenderIComponents() {
	for (IComponent* component : componentsList) {
		component->GetName();
		auto fields = component->IComponent::GetSerializedFields(component);
	}
}