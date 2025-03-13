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
	audioEngine = &AudioEngine::Instance();
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
	//ImGui_PS::renderDrawData(dcb, ImGui::GetDrawData()); // Remove this line for WIN
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
			FramerateUI().Draw(dt);
		}
		if (i == mainMenu) {
			menuOption = MainMenuUI().Draw();
		}
		if (i == audioSliders) {
			AudioSliders().Draw();
		}
		if (i == healthbar) {
			Healthbar().Draw(health);
		}
	}
}