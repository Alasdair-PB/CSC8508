#pragma once
#include "../OpenGLRendering/OGLRenderer.h"
#include "../AudioEngine/AudioEngine.h"
#include "imgui.h"
#include "../CSC8508CoreClasses/PushdownMachine.h"
#include "../CSC8508CoreClasses/PushdownState.h"
#include "AudioSliders.h"
#include "FramerateUI.h"
#include "MainMenuUI.h"
#include "Healthbar.h"
#include <list>

namespace NCL {
	namespace UI {
			class UISystem {
				std::list<int> uiList;

			public:
				static UISystem* GetInstance() { return instance; }
				static void Shutdown() { if (instance) { delete instance; } }

				virtual void StartFrame() = 0;
				virtual void EndFrame() = 0;

				void RenderFrame();

				void PushNewStack(UIElementsGroup* group, std::string name);

				void RemoveStack(std::string name);

				std::unordered_map<std::string, UIElementsGroup*>elementStacks;

			protected:
				UISystem();
				virtual ~UISystem();

				static UISystem* instance;
			};
		}
	}

