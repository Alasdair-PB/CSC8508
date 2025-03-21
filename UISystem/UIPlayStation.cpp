#ifdef USE_PS5
#include "UIPlayStation.h"
#include "../PS5Core/PS5MemoryAllocator.h"
#include "../PS5Core/AGCRenderer.h"

using namespace NCL;
using namespace UI;

UISystem* UISystem::instance = nullptr;

UIPlayStation::UIPlayStation(sce::Agc::DrawCommandBuffer* dcb, PS5::MemoryAllocator* allocator, uint32_t numCx) {
	this->dcb = dcb;

    const ImGui_PS::AllocateFunc af = [](void* allocator, size_t size, size_t alignment) {
        return (void*) static_cast<PS5::MemoryAllocator*>(allocator)->Allocate(size, alignment);
        };
    const ImGui_PS::ReleaseFunc rf = [](void*, void*) {
        return;
        };
	
    ImGui_PS::initialize(allocator, af, rf, numCx);
	controlData.hasGamePad = true;
	controlData.enableNavigation = true;
}

UIPlayStation::~UIPlayStation() {
	ImGui_PS::shutdown();
	//ImGui_ImplOpenGL3_Shutdown();
	//ImGui_ImplWin32_Shutdown();
}

void UIPlayStation::StartFrame() {
	ImGui_PS::newFrame(SCREENWIDTH, SCREENHEIGHT, controlData);
	//ImGui_ImplOpenGL3_NewFrame();
	//ImGui_ImplWin32_NewFrame();
	UISystem::StartFrame();
}

void UIPlayStation::EndFrame() {
	UISystem::EndFrame();
	//ImGui_PS::renderDrawData(dcb, ImGui::GetDrawData());
}

#endif // USE_PS5