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
using namespace CSC8508;

UISystem::UISystem(HWND handle) : uiWindow(handle) {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_InitForOpenGL(handle); // Remove this line for PS5
	ImGui_ImplOpenGL3_Init(); // Remove this line for PS5
	audioEngine = &AudioEngine::Instance();
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
	ImGui::SetNextWindowPos(ImVec2(50, 50));
	ImGui::SetNextWindowSize(ImVec2(120, 50));
	bool open = true;
	ImGui::Begin("Framerate", &open, ImGuiWindowFlags_NoResize);
	ImGui::Text(std::to_string(1.0f / dt).c_str());
	ImGui::End();
}

void UISystem::AudioSliders() {
	ImGui::SetNextWindowPos(ImVec2(800, 300));
	ImGui::SetNextWindowSize(ImVec2(500, 125));
	ImGui::Begin("Audio Sliders");
	
	masterVolume = audioEngine->GetChannelVolume(ChannelGroupType::MASTER);
	masterVolume = masterVolume * 100;
	ImGui::SliderFloat("Master Volume", &masterVolume, 0, 100, "%.0f");
	masterVolume = masterVolume / 100;
	audioEngine->SetChannelVolume(ChannelGroupType::MASTER, masterVolume);

	musicVolume = audioEngine->GetChannelVolume(ChannelGroupType::MUSIC);
	musicVolume = musicVolume * 100;
	ImGui::SliderFloat("Music Volume", &musicVolume, 0, 100, "%.0f");
	musicVolume = musicVolume / 100;
	audioEngine->SetChannelVolume(ChannelGroupType::MUSIC, musicVolume);

	sfxVolume = audioEngine->GetChannelVolume(ChannelGroupType::SFX);
	sfxVolume = sfxVolume * 100;
	ImGui::SliderFloat("SFX Volume", &sfxVolume, 0, 100, "%.0f");
	sfxVolume = sfxVolume / 100;
	audioEngine->SetChannelVolume(ChannelGroupType::SFX, sfxVolume);

	voiceVolume = audioEngine->GetChannelVolume(ChannelGroupType::CHAT);
	voiceVolume = voiceVolume * 100;
	ImGui::SliderFloat("Voice Volume", &voiceVolume, 0, 100, "%.0f");
	voiceVolume = voiceVolume / 100;
	audioEngine->SetChannelVolume(ChannelGroupType::CHAT, voiceVolume);

	ImGui::End();
}

int UISystem::MainMenu() {
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