#pragma once
#include "../OpenGLRendering/OGLRenderer.h"
#include "imgui.h"
#include "../CSC8508CoreClasses/PushdownMachine.h"
#include "../CSC8508CoreClasses/PushdownState.h"
#include <iostream>
#include <set>


namespace NCL {
	namespace UI {

		class UIElement {
		public:
			std::string elementName = "";
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

		class StatelessButtonElement : public UIElement {
		public:
			void SetFunc(std::function<void()> f) {
				func = f;
			}
			void SetName(std::string n) {
				buttonName = n;
			}
			void SetSize(ImVec2 s) {
				size = s;
			}
		protected:
			std::function<void()> func;
			std::string buttonName;
			ImVec2 size;
			CSC8508::PushdownState::PushdownResult UpdateElement()override {
				if (ImGui::Button(buttonName.c_str(), ImVec2(size.x * Window::GetWindow()->GetScreenSize().x, size.y * Window::GetWindow()->GetScreenSize().y)))
				{ func(); }
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

		class StaticTextElement : public UIElement {
		public:
			StaticTextElement(std::string text) : text(text) {}

			CSC8508::PushdownState::PushdownResult UpdateElement() override {
				ImGui::Text(text.c_str());
				return CSC8508::PushdownState::PushdownResult::NoChange;
			}
		protected:
			std::string text;
		};

		class TextElement : public UIElement {
		public:
			TextElement(std::string* text) :
				text(text) {
			}

			CSC8508::PushdownState::PushdownResult UpdateElement() override {
				if (!text) return CSC8508::PushdownState::PushdownResult::NoChange;
				ImGui::Text(text->c_str());
				return CSC8508::PushdownState::PushdownResult::NoChange;
			}
		protected:
			std::string* text;
		};

		class ToggleElement : public UIElement {
		public:
			ToggleElement(std::string name, bool* toggle, float scale) :
				toggle(toggle), name(name), scale(scale){
			}

			CSC8508::PushdownState::PushdownResult UpdateElement() override {
				if (!toggle) return CSC8508::PushdownState::PushdownResult::NoChange;

				ImGui::PushItemWidth(scale);
				ImGui::Text(name.c_str());
				ImGui::SameLine();

				if (*toggle)
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
				else
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));

				if (ImGui::Button("Toggle"))
					*toggle = !(*toggle);

				ImGui::PopStyleColor();
				ImGui::PopItemWidth();
				return CSC8508::PushdownState::PushdownResult::NoChange;
			}
		protected:
			bool* toggle;
			std::string name;
			float scale;
		};

		class QuaternionElement : public UIElement {
		public:
			QuaternionElement(Vector4* vector, float scale, std::string name) :
				vector(vector), scale(scale), name(name) {
			}

			CSC8508::PushdownState::PushdownResult UpdateElement() override {
				if (!vector) return CSC8508::PushdownState::PushdownResult::NoChange;
				float x = vector->x;
				float y = vector->y;
				float z = vector->z;
				float w = vector->w;
				ImGui::Text(name.c_str());
				ImGui::PushItemWidth(scale);
				ImGui::InputFloat("X", &x, 0.1f, 1.0f, "%.2f");
				ImGui::SameLine();
				ImGui::InputFloat("Y", &y, 0.1f, 1.0f, "%.2f");
				ImGui::SameLine();
				ImGui::InputFloat("Z", &z, 0.1f, 1.0f, "%.2f");
				ImGui::SameLine();
				ImGui::InputFloat("W", &w, 0.1f, 1.0f, "%.2f");
				ImGui::PopItemWidth();
				vector->x = x;
				vector->y = y;
				vector->z = z;
				vector->w = w;

				return CSC8508::PushdownState::PushdownResult::NoChange;
			}
		protected:
			Vector4* vector;
			float scale;
			std::string name;
		};

		class FloatElement : public UIElement {
		public:
			FloatElement(float* value, float scale, std::string name) :
				value(value), scale(scale), name(name) {
			}

			CSC8508::PushdownState::PushdownResult UpdateElement() override {
				if (!value) return CSC8508::PushdownState::PushdownResult::NoChange;
				ImGui::PushItemWidth(scale);
				ImGui::InputFloat(name.c_str(), value, 0.1f, 1.0f, "%.2f");
				ImGui::PopItemWidth();
				return CSC8508::PushdownState::PushdownResult::NoChange;
			}
		protected:
			float* value;
			float scale;
			std::string name;
		};

		class VectorElement : public UIElement {
		public:
			VectorElement(Vector3* vector, float scale, std::string name) :
				vector(vector), scale(scale), name(name) {
			}

			CSC8508::PushdownState::PushdownResult UpdateElement() override {
				if (!vector) return CSC8508::PushdownState::PushdownResult::NoChange;
				float x = vector->x;
				float y = vector->y;
				float z = vector->z;
				ImGui::Text(name.c_str());
				ImGui::PushItemWidth(scale);
				ImGui::InputFloat("X", &x, 0.1f, 1.0f, "%.2f");
				ImGui::SameLine();
				ImGui::InputFloat("Y", &y, 0.1f, 1.0f, "%.2f");
				ImGui::SameLine();
				ImGui::InputFloat("Z", &z, 0.1f, 1.0f, "%.2f");
				ImGui::PopItemWidth();
				vector->x = x;
				vector->y = y;
				vector->z = z;

				return CSC8508::PushdownState::PushdownResult::NoChange;
			}
		protected:
			Vector3* vector;
			float scale;
			std::string name;
		};

		class InputFieldElement : public UIElement {
		public:
			void SetName(std::string n) {
				fieldName = n;
			}
			void SetInput(std::string i) {
				input = i;
			}
			void SetFunc(std::function<CSC8508::PushdownState::PushdownResult(std::string)> f) {
				func = f;
			}
			CSC8508::PushdownState::PushdownResult UpdateElement()override {
				strncpy_s(inputData, input.c_str(), 64);
				ImGui::InputText(fieldName.c_str(), inputData, 64);
				input = std::string(inputData);
				return func(inputData);
			}
		protected:
			std::string fieldName;
			std::string input;
			char inputData[64];
			std::function<CSC8508::PushdownState::PushdownResult(std::string)> func;
		};

		class StatelessInputFieldElement : public UIElement {
		public:
			StatelessInputFieldElement(std::string name, std::string* input) :
			name(name), input(input){}

			CSC8508::PushdownState::PushdownResult UpdateElement()override {
				strncpy_s(inputData, input->c_str(), 64);
				ImGui::InputText(name.c_str(), inputData, 64);
				*input = std::string(inputData);
				return CSC8508::PushdownState::PushdownResult::NoChange;
			}
		protected:
			std::string name;
			std::string* input;
			char inputData[64];
		};

		class EnumElement : public UIElement {
			public:
				EnumElement(std::string name, std::vector<std::pair<int*, std::string>> options)
					: name(name), options(options) {
				}

				CSC8508::PushdownState::PushdownResult UpdateElement() override {
					int currentItem = *options.front().first;
					std::vector<const char*> labels;

					for (auto& option : options)
						labels.push_back(option.second.c_str());

					if (ImGui::Combo(name.c_str(), &currentItem, labels.data(), static_cast<int>(labels.size())))
						*options[currentItem].first = currentItem;
					return CSC8508::PushdownState::PushdownResult::NoChange;
				}
			private:
				std::string name;
				std::vector<std::pair<int*, std::string>> options;
			};

		class EnumVectorElement : public UIElement {
		public:
			EnumVectorElement(std::string name, std::vector<int> values, std::vector<std::pair<int, std::string>> volumeMap)
				: name(name), values(values), volumeMap(volumeMap) {
			}

			CSC8508::PushdownState::PushdownResult UpdateElement() override {
				ImGui::Text("%s:", name.c_str());
				ImGui::SameLine();
				std::string displayText;
				for (int val : values) {
					for (const auto& pair : volumeMap) {
						if (pair.first == val) {
							if (!displayText.empty()) {
								displayText += ", ";
							}
							displayText += pair.second;
							break;
						}
					}
				}
				ImGui::Text("%s", displayText.c_str());
				return CSC8508::PushdownState::PushdownResult::NoChange;
			}

		private:
			std::string name;
			std::vector<int> values;
			std::vector<std::pair<int, std::string>> volumeMap;
		};
	}
}