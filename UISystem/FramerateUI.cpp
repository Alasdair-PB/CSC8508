#include "FramerateUI.h"
#include <filesystem>  
#include <functional>

using namespace NCL;
using namespace UI;

FramerateUI::FramerateUI()  {
	std::function<CSC8508::PushdownState::PushdownResult()> func = [this]() -> CSC8508::PushdownState::PushdownResult {
		ImGui::Text(std::to_string(1.0f / dt).c_str());
		return CSC8508::PushdownState::PushdownResult::NoChange;
	};
	 
	frameUI->PushVoidElement(func);
}

FramerateUI::~FramerateUI() {
	delete frameUI;
}
