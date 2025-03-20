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
					if (ImGui::Button(buttonName.c_str(), size)) { return func(); }
					return CSC8508::PushdownState::PushdownResult::NoChange;
				}

			};

			class SliderElement : public UIElement {
				float& val;
				float valMax;
				float valMin;
				CSC8508::PushdownState::PushdownResult UpdateElement()override {
					return CSC8508::PushdownState::PushdownResult::NoChange;
				}
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

			CSC8508::PushdownState::PushdownResult Draw(float dt) {
				ImGui::SetNextWindowPos(pos);
				ImGui::SetNextWindowSize(size);
				bool open = true;
				ImGui::Begin(winName.c_str(), &open, flags);

				CSC8508::PushdownState::PushdownResult result = CSC8508::PushdownState::PushdownResult::NoChange;
				for (auto const& element : elements) {
					result = element->UpdateElement();
					if (result != CSC8508::PushdownState::PushdownResult::NoChange) {
						return result;
					}
					ImGui::SetCursorPosY(ImGui::GetCursorPosY() + elementsOffset);
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

			void PushSliderElement(float& val, float valMax, float valMin) {}

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
		};
	}
}