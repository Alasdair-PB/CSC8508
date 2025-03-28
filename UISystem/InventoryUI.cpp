#include "InventoryUI.h"
#include <filesystem>  
#include <functional>

using namespace NCL;
using namespace UI;

InventoryUI::InventoryUI() {
	/*std::function<CSC8508::PushdownState::PushdownResult()> func = [this]() -> CSC8508::PushdownState::PushdownResult {
		ImGui::Text("Empty");
		return CSC8508::PushdownState::PushdownResult::NoChange;
		};
	inventoryUI->PushVoidElement(func);*/
}

InventoryUI::~InventoryUI() {
	delete inventoryUI;
}

void InventoryUI::PushInventoryElement(std::function<CSC8508::PushdownState::PushdownResult()> f) {
	inventoryUI->PushVoidElement(f);
}