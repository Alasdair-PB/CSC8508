#include "FramerateUI.h"
#include <filesystem>        

using namespace NCL;
using namespace UI;

FramerateUI::FramerateUI()  {

}

FramerateUI::~FramerateUI() {
}

void FramerateUI::Draw(float dt, int winWidth, int winHeight) {
	ImGui::SetNextWindowPos(ImVec2(winWidth * 0.02, winHeight * 0.05));
	ImGui::SetNextWindowSize(ImVec2(winWidth * 0.06, winHeight * 0.05));
	bool open = true;
	ImGui::Begin("Framerate", &open, ImGuiWindowFlags_NoResize);
	ImGui::Text(std::to_string(1.0f / dt).c_str());
	ImGui::End();
}
