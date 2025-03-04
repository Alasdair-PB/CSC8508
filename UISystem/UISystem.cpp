#include "UISystem.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_opengl3.h"
#include "imgui.h"
#include <filesystem>

using namespace NCL;
using namespace CSC8508;

UISystem::UISystem(HWND handle) : uiWindow(handle) {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsCustom1();
	ImGui_ImplWin32_InitForOpenGL(handle);
	ImGui_ImplOpenGL3_Init();
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

void UISystem::DisplayFramerate(float dt) {
	ImGui::StyleColorsDark();
	ImGui::SetNextWindowPos(ImVec2(50, 50));
	ImGui::SetNextWindowSize(ImVec2(120, 50));
	bool open = true;
	ImGui::Begin("Framerate", &open, ImGuiWindowFlags_NoResize);
	ImGui::Text(std::to_string(1.0f / dt).c_str());
	ImGui::End();
}

void UISystem::AudioSliders() {
	ImGui::Begin("Audio Slider");
	enum AudioOptions { AudioOption1, AudioOption2, AudioOption3, AudioOptionCount };
	static int option = AudioOption1;
	const char* optionNames[AudioOptionCount] = { "Option1", "Option2", "Option3" };
	const char* optionName = (option >= 0 && option < AudioOptionCount) ? optionNames[option] : "Unknown";
	ImGui::SliderInt("Audio Options", &option, 0, AudioOptionCount - 1, optionName);
	ImGui::End();
}

int UISystem::MainMenu() {
	ImGui::StyleColorsDark();
	ImGui::SetNextWindowPos(ImVec2(80, 480));
	ImGui::SetNextWindowSize(ImVec2(600, 500));
	bool open = true;
	enum options {none, startOffline, startServer, StartClient};
	int option = 0;
	ImGui::Begin("Main Menu", &open, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize);
	ImGui::SetWindowFontScale(2);
	if (ImGui::Button("Start Offline", ImVec2(600, 50))) {
		option = startOffline;
	}
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 50);
	if (ImGui::Button("Start as Server", ImVec2(600, 50))) {
		option = startServer;
	}
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 50);
	if (ImGui::Button("Start as Client", ImVec2(600, 50))) {
		option = StartClient;
	}
	ImGui::End();
	return option;
}