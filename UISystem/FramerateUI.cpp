#include "FramerateUI.h"
#include <filesystem>        
#include <string>
using namespace NCL;
using namespace UI;

FramerateUI::FramerateUI()  {

}

FramerateUI::~FramerateUI() {
}

void FramerateUI::Draw(float dt) {
	ImGui::SetNextWindowPos(ImVec2(50, 50));
	ImGui::SetNextWindowSize(ImVec2(120, 50));
	bool open = true;
	ImGui::Begin("Framerate", &open, ImGuiWindowFlags_NoResize);
	ImGui::Text(std::to_string(1.0f / dt).c_str());
	ImGui::End();
}
