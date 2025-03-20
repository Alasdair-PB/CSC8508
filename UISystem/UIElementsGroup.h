#pragma once
#include "../OpenGLRendering/OGLRenderer.h"
#include "imgui.h"
#include "../CSC8508CoreClasses/PushdownMachine.h"
#include "../CSC8508CoreClasses/PushdownState.h"

namespace NCL {
	namespace UI {
		class UIElementsGroup : public CSC8508::PushdownState {
			CSC8508::PushdownState::PushdownResult EscapeResult(CSC8508::PushdownState** newState) {}

			class UIElement {
			public:
				std::string name;
				virtual CSC8508::PushdownState::PushdownResult UpdateElement() {
					return CSC8508::PushdownState::PushdownResult::NoChange;
				};
			};

			class VoidElement : public UIElement {
			public:
				void SetFunc(std::function<CSC8508::PushdownState::PushdownResult()> f) {
					func = f;
				}
			protected:
				std::function<CSC8508::PushdownState::PushdownResult()> func;
				CSC8508::PushdownState::PushdownResult UpdateElement()override { return func(); };
			};

			class ButtonElement : public UIElement {
			public:
				void SetFunc(std::function<CSC8508::PushdownState::PushdownResult()> f) {
					func = f;
				}
				void SetName(std::string n) {
					buttonName = n;
				}
				void SetSize(ImVec2 s) {
					size = s;
				}
			protected:
				std::function<CSC8508::PushdownState::PushdownResult()> func;
				std::string buttonName;
				ImVec2 size;
				CSC8508::PushdownState::PushdownResult UpdateElement()override { 
					if (ImGui::Button(buttonName.c_str(), ImVec2(size.x * Window::GetWindow()->GetScreenSize().x, size.y * Window::GetWindow()->GetScreenSize().y))) { return func(); }
					return CSC8508::PushdownState::PushdownResult::NoChange;
				}

			};

			class SliderElement : public UIElement {
			public:
				void SetName(std::string n) {
					sliderName = n;
				}
				void SetVal(float& v) {
					val = v;
				}
				void SetMax(float max) {
					valMax = max;
				}
				void SetMin(float min) {
					valMin = min;
				}
				void SetFunc(std::function<CSC8508::PushdownState::PushdownResult(float)> f) {
					func = f;
				}
				CSC8508::PushdownState::PushdownResult UpdateElement()override {
					ImGui::SliderFloat(sliderName.c_str(), &val, valMin, valMax, "%.0f");
					return func(val);
				}
			protected:
				std::string sliderName;
				float val;
				float valMax;
				float valMin;
				std::function<CSC8508::PushdownState::PushdownResult(float)> func;
			};

			class TextElement : public UIElement {
				CSC8508::PushdownState::PushdownResult UpdateElement()override {
					return CSC8508::PushdownState::PushdownResult::NoChange;
				}
			};

			std::vector<UIElement*> elements;

		public:
			UIElementsGroup(ImVec2 pos, ImVec2 size, float fontScale = 2, std::string name = "", float elementsOffset = 50, ImGuiWindowFlags flags = 0)
				: pos(pos), size(size), fontScale(fontScale), winName(name), elementsOffset(elementsOffset), flags(flags) {
			}

			void OnAwake() { return; }

			CSC8508::PushdownState::PushdownResult Draw() {
				winWidth = Window::GetWindow()->GetScreenSize().x;
				winHeight = Window::GetWindow()->GetScreenSize().y;

				ImGui::SetNextWindowPos(ImVec2(pos.x * winWidth, pos.y * winHeight));
				ImGui::SetNextWindowSize(ImVec2(size.x * winWidth, size.y * winHeight));
				bool open = true;
				ImGui::Begin(winName.c_str(), &open, flags);
				ImGui::SetWindowFontScale(fontScale);

				CSC8508::PushdownState::PushdownResult result = CSC8508::PushdownState::PushdownResult::NoChange;
				for (auto const& element : elements) {
					result = element->UpdateElement();
					if (result != CSC8508::PushdownState::PushdownResult::NoChange) {
						return result;
					}
					ImGui::SetCursorPosY(ImGui::GetCursorPosY() + elementsOffset * winHeight);
				}
				ImGui::End();
				return CSC8508::PushdownState::PushdownResult::NoChange;
			}

			void PushButtonElement(ImVec2 size, std::string name, 
				std::function<CSC8508::PushdownState::PushdownResult()> func) {
				ButtonElement* buttonElem = new ButtonElement;
				buttonElem->SetName(name);
				buttonElem->SetSize(size);
				buttonElem->SetFunc(func);
				elements.push_back(buttonElem);
			}

			void PushSliderElement(std::string name, float& val, float valMax, float valMin, 
				std::function<CSC8508::PushdownState::PushdownResult(float)> func) {
				SliderElement* sliderElem = new SliderElement;
				sliderElem->SetName(name);
				sliderElem->SetVal(val);
				sliderElem->SetMax(valMax);
				sliderElem->SetMin(valMin);
				sliderElem->SetFunc(func);
				elements.push_back(sliderElem);
			}

			void PushTextElement(std::string text) {}

			void PushVoidElement(std::function<CSC8508::PushdownState::PushdownResult()> func) {
				VoidElement* voidElem = new VoidElement;
				voidElem->SetFunc(func);
				elements.push_back(voidElem);
			}

			/*void PushEscapeElement() {
				PushButtonElement(ImVec2(50, 50), "Return", onAwake);

			}*/

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