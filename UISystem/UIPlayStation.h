#pragma once
#ifdef USE_PS5
#include "UISystem.h"
#include "imgui_impl_ps.h"
#include "../PS5Core/PS5MemoryAllocator.h"

namespace NCL {
	namespace UI {
		class UIPlayStation : public UISystem
		{
		public:
			static UIPlayStation* GetInstance() { return static_cast<UIPlayStation*>(UISystem::GetInstance()); }

			static void Initialize(sce::Agc::DrawCommandBuffer* dcb, PS5::MemoryAllocator* allocator, uint32_t numCx) {
				instance = (instance == nullptr) ? new UIPlayStation(dcb, allocator, numCx) : instance;
			}

			void UpdateDCB(sce::Agc::DrawCommandBuffer* dcb) {
				this->dcb = dcb;
			}
			void StartFrame() override;
			void EndFrame() override;

		protected:
			UIPlayStation(sce::Agc::DrawCommandBuffer* dcb, PS5::MemoryAllocator* allocator, uint32_t numCx);
			~UIPlayStation();

			sce::Agc::DrawCommandBuffer* dcb;
			PS5::MemoryAllocator allocator;

			ImGui_PS::ControlData controlData;
		
		};
	}
}

#endif // USE_PS5

