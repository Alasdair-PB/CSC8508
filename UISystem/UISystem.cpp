#include "UISystem.h"
#ifdef USE_PS5
#include "UIPlayStation.h"
#else
#include "UIWindows.h"
#endif // USE_PS5

#include "imgui.h"
#include <filesystem>

using namespace NCL;
using namespace UI;

class UIElementsGroup : public CSC8508::PushdownState {
	CSC8508::PushdownState::PushdownResult EscapeResult(CSC8508::PushdownState** newState){}

	class UIElement {
	public:
		std::string name;
		virtual CSC8508::PushdownState::PushdownResult UpdateElement();
	};

	class VoidElement : public UIElement {
		std::function<void()> func;
		CSC8508::PushdownState::PushdownResult UpdateElement();
	};

	class ButtonElement : public UIElement {
		std::function<void()> func;
		ImVec2 imVec;
		CSC8508::PushdownState::PushdownResult UpdateElement()override { if (ImGui::Button(name.c_str(), imVec)) {} }
	};

	class SliderElement : public UIElement {
		float& val;
		float valMax;
		float valMin;
		CSC8508::PushdownState::PushdownResult UpdateElement()override {

		}
	};

	class TextElement : public UIElement {

	};

	std::vector<UIElement*> elements;

public:
	UIElementsGroup(ImVec2 pos, ImVec2 size, float fontScale = 2, std::string name = "", float elementsOffset = 50, ImGuiWindowFlags flags = 0)
		: pos(pos), size(size), fontScale(fontScale), name(name.c_str()), elementsOffset(elementsOffset), flags(flags) {
	}

	void OnAwake() { return; }

	CSC8508::PushdownState::PushdownResult Draw(float dt, CSC8508::PushdownState** newState) {
		ImGui::SetNextWindowPos(pos);
		ImGui::SetNextWindowSize(size);
		bool open = true;
		ImGui::Begin(name, &open, flags);

		CSC8508::PushdownState::PushdownResult result = CSC8508::PushdownState::PushdownResult::NoChange;
		for (auto const& element : elements) {
			result = element->UpdateElement();
			if (result != CSC8508::PushdownState::PushdownResult::NoChange) {
				return result;
			}
		}
		return CSC8508::PushdownState::PushdownResult::NoChange;
	}

	void PushButtonElement(ImVec2 size, std::string name, std::function<void()> func) {

	}

	void PushSliderElement(float& val, float valMax, float valMin) {

	}

	void PushTextElement(std::string text) {

	}

	void PushVoidElement(std::function<void()> func) {

	}

	void PushEscapeElement() {
		PushButtonElement(ImVec2(50, 50), "Return", onAwake);
	}

protected:
	ImVec2 pos;
	ImVec2 size;
	float fontScale;
	const char* name;
	float elementsOffset;
	ImGuiWindowFlags flags;
};


UISystem::UISystem() {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
}

UISystem::~UISystem() {
	ImGui::DestroyContext();
}

void UISystem::StartFrame() {
	ImGui::NewFrame();
}

void UISystem::EndFrame() {
	ImGui::Render();
	ImGui::EndFrame();
}

void UISystem::DisplayWindow(int window) {
	uiList.push_back(window);
}

void UISystem::HideWindow(int window) {
	uiList.remove(window);
}

void UISystem::DrawWindows() {
	for (auto const& i : uiList) {
		if (i == framerate) {
			FramerateUI().Draw(dt, winWidth, winHeight);
		}
		if (i == mainMenu) {
			menuOption = MainMenuUI().Draw(winWidth, winHeight);
		}
		if (i == audioSliders) {
			AudioSliders().Draw(winWidth, winHeight);
		}
		if (i == healthbar) {
			Healthbar().Draw(health, winWidth, winHeight);
		}
	}
}