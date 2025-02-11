#pragma once
#include "OGLRenderer.h"

namespace NCL {
	namespace CSC8508 {
		class UISystem {
		public:
			UISystem(HWND handle);
			~UISystem();

			void SetWindow(HWND handle) {
				uiWindow = handle;
			}

			void DrawDemo();

		protected:
			HWND uiWindow;
			bool showDemo = true;
		};
	}
}

