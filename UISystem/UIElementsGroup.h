#pragma once
#include "../OpenGLRendering/OGLRenderer.h"
#include "../AudioEngine/AudioEngine.h"
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
				virtual CSC8508::PushdownState::PushdownResult UpdateElement();
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
				ImGui::End();
			}

			void PushButtonElement(ImVec2 size, std::string name, std::function<void()> func) {}

			void PushSliderElement(float& val, float valMax, float valMin) {}

			void PushTextElement(std::string text) {}

			void PushVoidElement(std::function<CSC8508::PushdownState::PushdownResult()> func) {
				VoidElement* voidElem = new VoidElement;
				voidElem->SetFunc(func);
				elements.push_back(voidElem);
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
	}
}