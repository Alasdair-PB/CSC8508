#include "MainMenuUI.h"
#include <filesystem>        
#include <iostream>

using namespace NCL;
using namespace UI;

MainMenuUI::MainMenuUI() {

}

MainMenuUI::~MainMenuUI() {

}

int MainMenuUI::DrawMainMenu() {

	ImGui::SetNextWindowPos(ImVec2(80, 480));
	ImGui::SetNextWindowSize(ImVec2(600, 500));
	bool open = true;
	ImGui::Begin("Main Menu", &open, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize);
	ImGui::SetWindowFontScale(2);
	if (ImGui::Button("Start Offline", ImVec2(600, 50))) {
		menuOption = startOffline;
	}
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 50);
	if (ImGui::Button("Start as Server", ImVec2(600, 50))) {
		menuOption = startServer;
	}
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 50);
	if (ImGui::Button("Start as Client", ImVec2(600, 50))) {
		menuOption = StartClient;
	}
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 50);
	if (ImGui::Button("Host Lobby", ImVec2(600, 50))) {
		menuOption = eosOption;
	}
	ImGui::End();
	return menuOption;
}

int MainMenuUI::DrawLobbyMenu() {
	ImGui::SetNextWindowPos(ImVec2(80, 480));
	ImGui::SetNextWindowSize(ImVec2(600, 500));
	bool open = true;
	ImGui::Begin("Lobby Menu", &open, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize);
	ImGui::SetWindowFontScale(2);
	if (ImGui::Button("Create Lobby", ImVec2(600, 50))) {
		menuOption = hostLobby;
	}
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 50);
	if (ImGui::Button("Join Lobby", ImVec2(600, 50))) {
		menuOption = joinLobby;
	}
	ImGui::End();
	return menuOption;
}

int MainMenuUI::DrawLobbyDetails() {
	ImGui::SetNextWindowPos(ImVec2(80, 480));
	ImGui::SetNextWindowSize(ImVec2(600, 500));
	bool open = true;
	ImGui::Begin("Lobby Menu", &open, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize);
	ImGui::SetWindowFontScale(2);
	if (ImGui::Button("Start", ImVec2(600, 50))) {
		menuOption = startLobbyGame;
	}
	ImGui::End();
	return menuOption;
}