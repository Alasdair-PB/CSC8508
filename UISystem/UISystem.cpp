#include "UISystem.h"
#include <filesystem>        

using namespace NCL;
using namespace CSC8508;

UISystem::UISystem(HWND handle) : uiWindow(handle) {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_InitForOpenGL(handle);
	ImGui_ImplOpenGL3_Init();
	audioEngine = &AudioEngine::Instance();
}

UISystem::~UISystem() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void UISystem::StartFrame() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void UISystem::EndFrame() {
	ImGui::Render();
	ImGui::EndFrame();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
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
			DisplayFramerate();
		}
		if (i == mainMenu) {
			MainMenu();
		}
		if (i == audioSliders) {
			AudioSliders();
		}
	}
}

void UISystem::DrawDemo() {
	ImGui::SetNextWindowPos(ImVec2(100, 100));
	ImGui::SetNextWindowSize(ImVec2(200, 100));
	ImGui::Begin("Test Window");
	if (ImGui::Button("Hide Demo Window")) {
		showDemo = false;
	}
	if (ImGui::Button("Show Demo Window")) {
		showDemo = true;
	}
	ImGui::End();
	if (showDemo == true) {
		ImGui::ShowDemoWindow();
	}
}