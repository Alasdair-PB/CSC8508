#include "UIElementsGroup.h"

using namespace NCL;
using namespace CSC8508;
using namespace UI;

UIElementsGroup::UIElementsGroup(ImVec2 pos, ImVec2 size, float fontScale, std::string name,
	float elementsOffset, ImGuiWindowFlags flags)
	: pos(pos), size(size), 
	fontScale(fontScale), 
	winName(name), 
	elementsOffset(elementsOffset), 
	flags(flags) {}

UIElementsGroup::~UIElementsGroup() {
	for (UIElement* element : elements)
		delete element;
	elements.clear();
}

void UIElementsGroup::RemoveElementsFromIndex(int cutOff) {
	if (cutOff >= elements.size()) return;
	for (size_t i = cutOff; i < elements.size(); i++)
		delete elements[i];
	elements.erase(elements.begin() + cutOff, elements.end());
}

CSC8508::PushdownState::PushdownResult UIElementsGroup::Draw() {
	winWidth = Window::GetWindow()->GetScreenSize().x;
	winHeight = Window::GetWindow()->GetScreenSize().y;

	ImGui::SetNextWindowPos(ImVec2(pos.x * winWidth, pos.y * winHeight));
	ImGui::SetNextWindowSize(ImVec2(size.x * winWidth, size.y * winHeight));
	bool open = true;
	ImGui::Begin(winName.c_str(), &open, flags);
	ImGui::SetWindowFontScale(fontScale);

	CSC8508::PushdownState::PushdownResult result = CSC8508::PushdownState::PushdownResult::NoChange;
	for (int i = 0; i < elements.size(); i++) {
		auto element = elements[i];
		ImGui::PushID(i);
		result = element->UpdateElement();
		if (result != CSC8508::PushdownState::PushdownResult::NoChange)
			return result;
		ImGui::PopID();
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + elementsOffset * winHeight);
	}
	ImGui::End();
	return CSC8508::PushdownState::PushdownResult::NoChange;
}

void UIElementsGroup::PushToggle(std::string name, bool* toggle, float scale) {
	ToggleElement* boolFieldElem = new ToggleElement(name, toggle, scale * winWidth);
	elements.push_back(boolFieldElem);
}

void UIElementsGroup::PushButtonElement(ImVec2 size, std::string name,
	std::function<CSC8508::PushdownState::PushdownResult()> func) {
	ButtonElement* buttonElem = new ButtonElement;
	buttonElem->SetName(name);
	buttonElem->SetSize(size);
	buttonElem->SetFunc(func);
	elements.push_back(buttonElem);
}

void UIElementsGroup::PushEnumElement(std::string name, std::set<std::pair<int*, std::string>> pair) {
	EnumElement* enumElem = new EnumElement(name, pair);
	elements.push_back(enumElem);
}

void UIElementsGroup::PushStatelessButtonElement(ImVec2 size, std::string name,
	std::function<void()> func) {
	StatelessButtonElement* buttonElem = new StatelessButtonElement;
	buttonElem->SetName(name);
	buttonElem->SetSize(size);
	buttonElem->SetFunc(func);
	elements.push_back(buttonElem);
}

void UIElementsGroup::PushSliderElement(std::string name, float& val, float valMax, float valMin,
	std::function<CSC8508::PushdownState::PushdownResult(float)> func) {
	SliderElement* sliderElem = new SliderElement;
	sliderElem->SetName(name);
	sliderElem->SetVal(val);
	sliderElem->SetMax(valMax);
	sliderElem->SetMin(valMin);
	sliderElem->SetFunc(func);
	elements.push_back(sliderElem);
}

void UIElementsGroup::PushVectorElement(Vector3* value, float scale) {
	VectorElement* vectorElem = new VectorElement(value, scale * winWidth);
	elements.push_back(vectorElem);
}

void UIElementsGroup::PushFloatElement(float* value, float scale, std::string name) {
	FloatElement* floatElem = new FloatElement(value, scale * winWidth, name);
	elements.push_back(floatElem);
}

void UIElementsGroup::PushQuaternionElement(Vector4* value, float scale) {
	QuaternionElement* quatElem = new QuaternionElement(value, scale * winWidth);
	elements.push_back(quatElem);
}

void UIElementsGroup::PushTextElement(std::string* text) {
	TextElement* inputFieldElem = new TextElement(text);
	elements.push_back(inputFieldElem);
}

void UIElementsGroup::PushStaticTextElement(std::string text) {
	StaticTextElement* inputFieldElem = new StaticTextElement(text);
	elements.push_back(inputFieldElem);
}

void UIElementsGroup::PushInputFieldElement(std::string name, std::string input, std::function<CSC8508::PushdownState::PushdownResult(std::string)> func) {
	InputFieldElement* inputFieldElem = new InputFieldElement;
	inputFieldElem->SetName(name);
	inputFieldElem->SetInput(input);
	inputFieldElem->SetFunc(func);
	elements.push_back(inputFieldElem);
}

void UIElementsGroup::PushStatelessInputFieldElement(std::string name, std::string* input) {
	StatelessInputFieldElement* inputFieldElem = new StatelessInputFieldElement(name, input);
	elements.push_back(inputFieldElem);
}

void UIElementsGroup::PushVoidElement(std::function<CSC8508::PushdownState::PushdownResult()> func) {
	VoidElement* voidElem = new VoidElement;
	voidElem->SetFunc(func);
	elements.push_back(voidElem);
}