#include "FramerateUI.h"
#include <filesystem>  
#include <functional>

using namespace NCL;
using namespace UI;

FramerateUI::FramerateUI()  {
	UIElementsGroup* frameUI = new UIElementsGroup(ImVec2(50, 50), ImVec2(120, 50), 1.0f, "Framerate");

	std::function<CSC8508::PushdownState::PushdownResult()> func = [this]() -> CSC8508::PushdownState::PushdownResult {
		ImGui::Text(std::to_string(1.0f / dt).c_str());
		return CSC8508::PushdownState::PushdownResult::NoChange;
	};
	 
	frameUI->PushVoidElement(func);
}

FramerateUI::~FramerateUI() {

}

CSC8508::PushdownState::PushdownResult FramerateUI::OnElementsRender(float dt) {
	ImGui::Text(std::to_string(1.0f / dt).c_str());
	return CSC8508::PushdownState::PushdownResult::NoChange;
}

/*void FramerateUI::Draw(float dt, int winWidth, int winHeight) {
	ImGui::SetNextWindowPos(ImVec2(winWidth * 0.02, winHeight * 0.05));
	ImGui::SetNextWindowSize(ImVec2(winWidth * 0.06, winHeight * 0.05));
	bool open = true;
	ImGui::Begin("Framerate", &open, ImGuiWindowFlags_NoResize);
	ImGui::Text(std::to_string(1.0f / dt).c_str());
	ImGui::End();
}*/
