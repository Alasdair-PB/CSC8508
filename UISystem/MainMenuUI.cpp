#include "MainMenuUI.h"
#include <filesystem>        

using namespace NCL;
using namespace UI;

MainMenuUI::MainMenuUI() {

}

MainMenuUI::~MainMenuUI() {

}

int MainMenuUI::Draw(int winWidth, int winHeight) {
	ImGui::SetNextWindowPos(ImVec2(winWidth * 0.1, winHeight * 0.5));
	ImGui::SetNextWindowSize(ImVec2(600, 500));
	bool open = true;
	ImGui::Begin("Main Menu", &open, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize);
	ImGui::SetWindowFontScale(winWidth * 0.001);
	if (winHeight > winWidth) {
		ImGui::SetWindowFontScale(winHeight * 0.001);
	}
	if (ImGui::Button("Start Offline", ImVec2(winWidth * 0.3, winHeight * 0.05))) {
		menuOption = startOffline;
	}
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (winHeight * 0.05));
	if (ImGui::Button("Start as Server", ImVec2(winWidth * 0.3, winHeight * 0.05))) {
		menuOption = startServer;
	}
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (winHeight * 0.05));
	if (ImGui::Button("Start as Client", ImVec2(winWidth * 0.3, winHeight * 0.05))) {
		menuOption = StartClient;
	}
	ImGui::End();
	return menuOption;
}