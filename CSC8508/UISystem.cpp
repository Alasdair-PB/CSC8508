#include "UISystem.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_opengl3.h"
#include "ImGui/imgui.h"

using namespace NCL;
using namespace CSC8508;

UISystem::UISystem(HWND handle) : uiWindow(handle) {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_InitForOpenGL(handle);
	ImGui_ImplOpenGL3_Init();
}

UISystem::~UISystem() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void UISystem::DrawDemo() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

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

	ImGui::Render();
	ImGui::EndFrame();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}