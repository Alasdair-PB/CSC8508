#include "UISystem.h"
#ifdef USE_PS5
#include "UIPlayStation.h"
#else
#include "UIWindows.h"
#endif // USE_PS5

#include "imgui.h"
#include <filesystem>

using namespace NCL;
using namespace UI;

UISystem::UISystem() {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
}

UISystem::~UISystem() {
	ImGui::DestroyContext();
}

void UISystem::StartFrame() {
	ImGui::NewFrame();
}

void UISystem::EndFrame() {
	ImGui::Render();
	ImGui::EndFrame();
}

void UISystem::DisplayWindow(int window) {
	uiList.push_back(window);
}

void UISystem::HideWindow(int window) {
	uiList.remove(window);
}

void UISystem::DrawWindows() {
	for (auto const& i : uiList) {
		if (i == framerate) {
			FramerateUI().Draw(dt, winWidth, winHeight);
		}
		if (i == mainMenu) {
			menuOption = MainMenuUI().Draw(winWidth, winHeight);
		}
		if (i == audioSliders) {
			AudioSliders().Draw(winWidth, winHeight);
		}
		if (i == healthbar) {
			Healthbar().Draw(health, winWidth, winHeight);
		}
	}
}