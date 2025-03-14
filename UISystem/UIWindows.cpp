#ifndef USE_PS5
#include "UIWindows.h"
#include <filesystem>    
#include "imgui_impl_win32.h"
#include "imgui_impl_opengl3.h"

using namespace NCL;
using namespace UI;

UISystem* UISystem::instance = nullptr;

UIWindows::UIWindows(HWND handle) : handle(handle) {
	ImGui_ImplWin32_InitForOpenGL(handle);
	ImGui_ImplOpenGL3_Init();
}

UIWindows::~UIWindows() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplWin32_Shutdown();
}

void UIWindows::StartFrame() {
	ImGui_ImplOpenGL3_NewFrame(); 
	ImGui_ImplWin32_NewFrame();
	UISystem::StartFrame();
}

void UIWindows::EndFrame() {
	UISystem::EndFrame();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
#endif // USE_PS5