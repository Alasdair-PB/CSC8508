#include "UISystem.h"
#ifdef USE_PS5
#include "UIPlayStation.h"
#else
#include "UIWindows.h"
#endif // USE_PS5

#include "imgui.h"
#include <filesystem>

using namespace NCL;
using namespace UI;

UISystem::UISystem() {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
}

UISystem::~UISystem() {
	ImGui::DestroyContext();
}

void UISystem::StartFrame() {
	ImGui::NewFrame();
}

void UISystem::EndFrame() {
	ImGui::Render();
	ImGui::EndFrame();
}

void UISystem::RenderFrame() {
	for (const auto& element : elementStacks) {
		element.second->Draw(dt);
	}
}

void UISystem::PushNewStack(UIElementsGroup* group, std::string name) {
	elementStacks[name] = group;
}

void UISystem::RemoveStack(std::string name) {
	elementStacks.erase(name);
}