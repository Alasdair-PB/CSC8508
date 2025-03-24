#pragma once
#include "imgui.h"
#include "UIElementsGroup.h"

namespace NCL {
	namespace UI {

		class InventoryUI {
		public:
			InventoryUI();
			~InventoryUI();

			void LoadTex();

			void SetTex(Texture* tex) {
				invTex = tex;
			}

			UIElementsGroup* inventoryUI = new UIElementsGroup(ImVec2(0.3f, 0.1f), ImVec2(0.2f, 0.2f), 1.0f, "Inventory", 0.0f, ImGuiWindowFlags_NoResize);

		protected:
			Texture* invTex;
		};
	}
}