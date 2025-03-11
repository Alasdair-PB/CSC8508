#include "UISystem.h"
#ifdef USE_PS5
#include "imgui_impl_ps.h"
#else
#include "imgui_impl_win32.h"
#include "imgui_impl_opengl3.h"
#endif // USE_PS5

#include "imgui.h"
#include <filesystem>

using namespace NCL;
using namespace UI;

UISystem::UISystem(HWND handle) : uiWindow(handle) {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_InitForOpenGL(handle); // Remove this line for PS5
	ImGui_ImplOpenGL3_Init(); // Remove this line for PS5
}

UISystem::~UISystem() {
	ImGui_ImplOpenGL3_Shutdown(); // Remove this line for PS5
	ImGui_ImplWin32_Shutdown(); // Remove this line for PS5
	ImGui::DestroyContext();
}

void UISystem::StartFrame() {
	ImGui_ImplOpenGL3_NewFrame(); // Remove this line for PS5
	ImGui_ImplWin32_NewFrame(); // Remove this line for PS5
	ImGui::NewFrame();
}

void UISystem::EndFrame() {
	ImGui::Render();
	ImGui::EndFrame();
	//ImGui_PS::renderDrawData(dcb, ImGui::GetDrawData()); // Remove this line for WIN
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());  // Remove this line for PS5
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
			NCL::CSC8508::FramerateUI::FramerateUI().Draw(dt);
		}
		if (i == mainMenu) {
			menuOption = NCL::CSC8508::MainMenuUI::MainMenuUI().Draw();
		}
		if (i == audioSliders) {
			NCL::CSC8508::AudioSliders::AudioSliders().Draw();
		}
		if (i == healthbar) {
			NCL::CSC8508::Healthbar::Healthbar().Draw(health);
		}
	}
}