#ifdef USE_PS5
#include "UIPlayStation.h"
#include "imgui_impl_ps.h"

using namespace NCL;
using namespace UI;

UISystem* UISystem::instance = nullptr;

UIPlayStation::UIPlayStation() {
	//ImGui_ImplWin32_InitForOpenGL(handle);
	//ImGui_ImplOpenGL3_Init();
}

UIPlayStation::~UIPlayStation() {
	//ImGui_ImplOpenGL3_Shutdown();
	//ImGui_ImplWin32_Shutdown();
}

void UIPlayStation::StartFrame() {
	//ImGui_ImplOpenGL3_NewFrame();
	//ImGui_ImplWin32_NewFrame();
	UISystem::StartFrame();
}

void UIPlayStation::EndFrame() {
	UISystem::EndFrame();
	//ImGui_PS::renderDrawData(dcb, ImGui::GetDrawData());
}
#endif // USE_PS5