#include "InventoryUI.h"
#include <filesystem>  
#include <functional>

using namespace NCL;
using namespace UI;

InventoryUI::InventoryUI() {
	std::function<CSC8508::PushdownState::PushdownResult()> func = [this]() -> CSC8508::PushdownState::PushdownResult {
		/*for (auto const& item : inventory.storedItems) {

		}*/

		return CSC8508::PushdownState::PushdownResult::NoChange;
		};

	inventoryUI->PushVoidElement(func);
}

InventoryUI::~InventoryUI() {
	delete inventoryUI;
}