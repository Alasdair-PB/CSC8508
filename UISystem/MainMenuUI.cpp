#include "MainMenuUI.h"
#include <filesystem>        

using namespace NCL;
using namespace UI;

MainMenuUI::MainMenuUI() {
	std::function<CSC8508::PushdownState::PushdownResult()> funcA = [this]() -> CSC8508::PushdownState::PushdownResult {
		menuOption = startOffline;
		return CSC8508::PushdownState::PushdownResult::NoChange;
	};

	std::function<CSC8508::PushdownState::PushdownResult()> funcB = [this]() -> CSC8508::PushdownState::PushdownResult {
		menuOption = startServer;
		return CSC8508::PushdownState::PushdownResult::NoChange;
	};

	std::function<CSC8508::PushdownState::PushdownResult()> funcC = [this]() -> CSC8508::PushdownState::PushdownResult {
		menuOption = startClient;
		return CSC8508::PushdownState::PushdownResult::NoChange;
	};

	menuUI->PushButtonElement(ImVec2(300, 50), "Start Offline", funcA);
	menuUI->PushButtonElement(ImVec2(300, 50), "Start Server", funcB);
	menuUI->PushButtonElement(ImVec2(300, 50), "Start Client", funcC);
}

MainMenuUI::~MainMenuUI() {
	delete menuUI;
}

//int MainMenuUI::Draw(int winWidth, int winHeight) {
//	ImGui::SetNextWindowPos(ImVec2(winWidth * 0.1, winHeight * 0.5));
//	ImGui::SetNextWindowSize(ImVec2(600, 500));
//	bool open = true;
//	ImGui::Begin("Main Menu", &open, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize);
//	ImGui::SetWindowFontScale(winWidth * 0.001);
//	if (winHeight > winWidth) {
//		ImGui::SetWindowFontScale(winHeight * 0.001);
//	}
//	if (ImGui::Button("Start Offline", ImVec2(winWidth * 0.3, winHeight * 0.05))) {
//		menuOption = startOffline;
//	}
//	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (winHeight * 0.05));
//	if (ImGui::Button("Start as Server", ImVec2(winWidth * 0.3, winHeight * 0.05))) {
//		menuOption = startServer;
//	}
//	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (winHeight * 0.05));
//	if (ImGui::Button("Start as Client", ImVec2(winWidth * 0.3, winHeight * 0.05))) {
//		menuOption = startClient;
//	}
//	ImGui::End();
//	return menuOption;
//}