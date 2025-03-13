#include "Healthbar.h"
#include <filesystem>        

using namespace NCL;
using namespace UI;

Healthbar::Healthbar() {

}

Healthbar::~Healthbar() {
}

void Healthbar::Draw(int health, int winWidth, int winHeight) {
	ImGui::SetNextWindowPos(ImVec2(winWidth * 0.1, winHeight * 0.9));
	ImGui::SetNextWindowSize(ImVec2(400, 100));
	bool open = true;
	ImGui::Begin("Health", &open, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize);
	float healthPerc = health / static_cast<float>(100);

	ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(1.00f, 0.00f, 0.00f, 1.00f));
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.00f, 0.00f, 0.00f, 0.00f));
	ImGui::SetCursorPosY(0);
	ImGui::ProgressBar(100, ImVec2(winWidth * 0.2, winHeight * 0.025));
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();

	ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.00f, 1.00f, 0.00f, 1.00f));
	ImGui::SetCursorPosY(0);
	ImGui::ProgressBar(healthPerc, ImVec2(winWidth * 0.2, winHeight * 0.025));
	ImGui::PopStyleColor();
	ImGui::End();
}