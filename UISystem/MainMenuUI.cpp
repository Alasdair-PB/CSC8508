#include "UISystem.h"
#include <filesystem>        

using namespace NCL;
using namespace CSC8508;


void UISystem::MainMenu() {
	ImGui::SetNextWindowPos(ImVec2(80, 480));
	ImGui::SetNextWindowSize(ImVec2(600, 500));
	bool open = true;
	ImGui::Begin("Main Menu", &open, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize);
	ImGui::SetWindowFontScale(2);
	if (ImGui::Button("Start Offline", ImVec2(600, 50))) {
		uiList.remove(audioSliders);
		menuOption = startOffline;
	}
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 50);
	if (ImGui::Button("Start as Server", ImVec2(600, 50))) {
		uiList.remove(audioSliders);
		menuOption = startServer;
	}
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 50);
	if (ImGui::Button("Start as Client", ImVec2(600, 50))) {
		uiList.remove(audioSliders);
		menuOption = StartClient;
	}
	ImGui::End();
}