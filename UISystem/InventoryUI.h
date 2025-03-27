#ifndef INVENTORYUI_H
#define INVENTORYUI_H

#include "UIElementsGroup.h"
#include <vector>

using namespace NCL::CSC8508;

namespace NCL {
	namespace UI {
		class InventoryUI {
		public:
			InventoryUI();
			~InventoryUI();

			/*void SetFunc(std::function<CSC8508::PushdownState::PushdownResult()> f) {
				func = f;
			}*/

			void PushInventoryElement(std::function<CSC8508::PushdownState::PushdownResult()> func);

			UIElementsGroup* inventoryUI = new UIElementsGroup(ImVec2(0.05f, 0.5f), ImVec2(0.1f, 0.3f), 1.0f, "Inventory", 0.1f, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);

		protected:
			/*std::vector<ItemComponent*> inventory;*/
			/*std::function<CSC8508::PushdownState::PushdownResult()> func;*/
		};
	}
}
#endif //INVENTORYUI_H