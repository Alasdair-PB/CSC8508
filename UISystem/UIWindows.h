#pragma once
#ifndef USE_PS5
#include "UISystem.h"
#include <windows.h>

namespace NCL {
	namespace UI {
		class UIWindows : public UISystem
		{
		public:
			static UIWindows* GetInstance() { return static_cast<UIWindows*>(UISystem::GetInstance());}

			static void Initialize(HWND handle) { 
				instance = (instance == nullptr) ? new UIWindows(handle) : instance; 
			}

			void StartFrame() override;
			void EndFrame() override;
			
		protected:
			UIWindows(HWND handle);
			~UIWindows();

			HWND handle;
		};
	}
}
#endif // USE_PS5
