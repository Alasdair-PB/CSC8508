#ifndef INVENTORYUI_H
#define INVENTORYUI_H

#include "imgui.h"
#include "UIElementsGroup.h"
#include <vector>

using namespace NCL::CSC8508;

namespace NCL {
	namespace UI {
		class InventoryUI {
		public:
			InventoryUI();
			~InventoryUI();

			UIElementsGroup* inventoryUI = new UIElementsGroup(ImVec2(0.3f, 0.1f), ImVec2(0.2f, 0.2f), 1.0f, "Inventory", 0.0f, ImGuiWindowFlags_NoResize);

		protected:
			/*std::vector<ItemComponent*> inventory;*/
		};
	}
}
#endif //INVENTORYUI_H