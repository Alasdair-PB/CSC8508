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
	audioEngine = &AudioEngine::Instance();
}

UIWindows::~UIWindows() {
	ImGui_ImplWin32_InitForOpenGL(handle);
	ImGui_ImplOpenGL3_Init();
	audioEngine = &AudioEngine::Instance();
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