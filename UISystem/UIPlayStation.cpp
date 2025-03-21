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

}

UIPlayStation::~UIPlayStation() {
	ImGui_PS::shutdown();
	sceSysmoduleUnloadModule(mouseModule);
}

void UIPlayStation::StartFrame() {
	ImGui_PS::ControlData controlData;
	controlData.drawCursor = true;

	ScePadData data;
	int ret = scePadReadState(padHandle, &data);
	ret = sceMouseRead(mouse_handle, mdata, 1);
	
	ImGui_PS::translate(&data, ImGui_PS::PadUsage_Navigation, mdata, lastMousePosition, &controlData); //INPUTS

	ImGui_PS::newFrame(SCREENWIDTH, SCREENHEIGHT, controlData);
	UISystem::StartFrame();
	lastMousePosition = ImGui::GetIO().MousePos;
}

void UIPlayStation::EndFrame() {
	UISystem::EndFrame();
	ImGui_PS::renderDrawData(*dcb, ImGui::GetDrawData());
}

void UIPlayStation::InitMouse(SceUserServiceUserId id) {
	mouseModule = sceSysmoduleLoadModule(SCE_SYSMODULE_MOUSE);
	SCE_AGC_ASSERT(mouseModule == SCE_OK);

	int ret = sceMouseInit();
	SCE_AGC_ASSERT(ret == SCE_OK);

	mouse_handle = sceMouseOpen(id, SCE_MOUSE_PORT_TYPE_STANDARD, 0, nullptr);

	lastMousePosition = ImVec2(
		float(SCREENWIDTH / 2) / 1080,
		float(SCREENHEIGHT / 2) / 1920);
}

#endif // USE_PS5