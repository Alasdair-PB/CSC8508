#pragma once
#ifdef USE_PS5
#include <libsysmodule.h>
#include "UISystem.h"
#include "imgui_impl_ps.h"
#include "../PS5Core/PS5MemoryAllocator.h"
#include "../include/mouse.h"

namespace NCL {
	namespace UI {
		class UIPlayStation : public UISystem
		{
		public:
			static UIPlayStation* GetInstance() { return static_cast<UIPlayStation*>(UISystem::GetInstance()); }

			static void Initialize(sce::Agc::DrawCommandBuffer* dcb, PS5::MemoryAllocator* allocator, uint32_t numCx) {
				instance = (instance == nullptr) ? new UIPlayStation(dcb, allocator, numCx) : instance;
			}

			void InitMouse(SceUserServiceUserId);

			void UpdateDCB(sce::Agc::DrawCommandBuffer* dcb) {
				this->dcb = dcb;
			}
			void SetPadHandle(uint32_t handle) {
				padHandle = padHandle == -99 ? handle : padHandle;
			}

			void StartFrame() override;
			void EndFrame() override;

		protected:
			UIPlayStation(sce::Agc::DrawCommandBuffer* dcb, PS5::MemoryAllocator* allocator, uint32_t numCx);
			~UIPlayStation();

			sce::Agc::DrawCommandBuffer* dcb;
			PS5::MemoryAllocator allocator;

			uint32_t padHandle = -99;
			uint32_t mouse_handle = -99;
			SceMouseData mdata[8];
			//ImGui_PS::ControlData controlData;
			ImVec2 lastMousePosition;

			uint16_t  mouseModule;
		
		};
	}
}

#endif // USE_PS5

