#pragma once
#include "../OpenGLRendering/OGLRenderer.h"
#include "imgui.h"
#include "UIElements.h"
#include "../CSC8508CoreClasses/PushdownMachine.h"
#include "../CSC8508CoreClasses/PushdownState.h"
#include <iostream>

namespace NCL {
	namespace UI {
		class UIElementsGroup : public CSC8508::PushdownState {
			CSC8508::PushdownState::PushdownResult EscapeResult(CSC8508::PushdownState** newState) {}

			std::vector<UIElement*> elements;

		public:
			UIElementsGroup(ImVec2 pos, ImVec2 size, float fontScale = 2, std::string name = " ",
				float elementsOffset = 50, ImGuiWindowFlags flags = 0);

			~UIElementsGroup();
			void RemoveElementsFromIndex(int cutOff);
			void ClearAllElements();
			void OnAwake() { return; }
			CSC8508::PushdownState::PushdownResult Draw();

			void PushButtonElement(ImVec2 size, std::string name,
				std::function<CSC8508::PushdownState::PushdownResult()> func);
			void PushSliderElement(std::string name, float& val, float valMax, float valMin,
				std::function<CSC8508::PushdownState::PushdownResult(float)> func);
			void PushInputFieldElement(std::string name, std::string input, 
				std::function<CSC8508::PushdownState::PushdownResult(std::string)> func);
			void PushVoidElement(std::function<CSC8508::PushdownState::PushdownResult()> func);
			void PushStatelessInputFieldElement(std::string name, std::string* input);

			void PushStatelessButtonElement(ImVec2 size, std::string name, std::function<void()> func);
			void PushToggle(std::string name, bool* toggle, float scale);
			void PushVectorElement(Vector3* value, float scale, std::string name);
			void PushFloatElement(float* value, float scale, std::string name);
			void PushEnumElement(std::string name, std::vector<std::pair<int*, std::string>> pair);
			void PushQuaternionElement(Vector4* value, float scale, std::string name);
			void PushEnumVectorElement(std::string name, std::vector<int> values, std::vector<std::pair<int, std::string>> enumMap);

			void PushTextElement(std::string* text);
			void PushStaticTextElement(std::string text);

		protected:
			ImVec2 pos;
			ImVec2 size;
			float fontScale;
			std::string winName;
			float elementsOffset;
			ImGuiWindowFlags flags;
			int winWidth = Window::GetWindow()->GetScreenSize().x;
			int winHeight = Window::GetWindow()->GetScreenSize().y;
		};
	}
}