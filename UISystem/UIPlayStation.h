#pragma once
#ifdef USE_PS5
#include "UISystem.h"

namespace NCL {
	namespace UI {
		class UIPlayStation : public UISystem
		{
		public:
			static UIPlayStation* GetInstance() { return static_cast<UIPlayStation*>(UISystem::GetInstance()); }

			static void Initialize() {
				instance = (instance == nullptr) ? new UIPlayStation() : instance;
			}

			void StartFrame() override;
			void EndFrame() override;

		protected:
			UIPlayStation();
			~UIPlayStation();
		
		};
	}
}

#endif // USE_PS5

