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

			void StartFrame();
			void EndFrame();

			void DrawDemo();
			void DisplayFramerate(float dt); 

		protected:
			HWND uiWindow;
			bool showDemo = true;
		};
	}
}

