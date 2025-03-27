#include "Healthbar.h"
#include <filesystem>        

using namespace NCL;
using namespace UI;

Healthbar::Healthbar() {
	std::function<CSC8508::PushdownState::PushdownResult()> func = [this]() -> CSC8508::PushdownState::PushdownResult {
		float healthPerc = health / static_cast<float>(100);
	
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(1.00f, 0.00f, 0.00f, 1.00f));
		ImGui::SetCursorPosY(0);
		ImGui::ProgressBar(100, ImVec2(0.2 * Window::GetWindow()->GetScreenSize().x, 0.03 * Window::GetWindow()->GetScreenSize().y));
		ImGui::PopStyleColor();
		
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.00f, 0.60f, 0.00f, 1.00f));
		ImGui::SetCursorPosY(0);
		ImGui::ProgressBar(healthPerc, ImVec2(0.2 * Window::GetWindow()->GetScreenSize().x, 0.03 * Window::GetWindow()->GetScreenSize().y));
		ImGui::PopStyleColor();
		return CSC8508::PushdownState::PushdownResult::NoChange;
	};

	healthbar->PushVoidElement(func);
}

Healthbar::~Healthbar() {
	delete healthbar;
}