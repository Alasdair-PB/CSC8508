#include "Debug.h"

#include "TutorialGame.h"
#include "NetworkedGame.h"

using namespace NCL;
using namespace CSC8508;

#ifdef USE_PS5
#include "./PS5/GameTechAGCRenderer.h"
#include "PS5Window.h"
#else
#include "Window.h"
#include "GameTechRenderer.h"
#endif // USE_PS5

#include <chrono>
#include <thread>
#include <sstream>
#include <functional>
#include <cstdlib>

void UpdateWindow(Window* w, NetworkedGame* g)
{
	float dt = w->GetTimer().GetTimeDeltaSeconds();
	w->SetTitle("Gametech frame time:" + std::to_string(std::roundf(1000.0f * dt)));
	g->UpdateGame(dt);
	Debug::UpdateRenderables(dt);
}

int main(int argc, char** argv) 
{
	int wWidth	= 1920;
	int wHeight = 1200;
	if (argc == 3) {
		std::cout << "AHK Launch" << "\n";
		wWidth = std::atoi(argv[1]);
		wHeight = std::atoi(argv[2]);
	}
	else {
		std::cout << "Normal Launch" << "\n";
	}
	

#ifdef USE_PS5
	std::unique_ptr<PS5::PS5Window> w = std::make_unique<PS5::PS5Window>("PS5", 1920, 1080);

	NetworkedGame* g = new NetworkedGame();

	while (w->UpdateWindow()) {
		float dt = w->GetTimer().GetTimeDeltaSeconds();
		g->UpdateGame(dt);
		//Debug::UpdateRenderables(dt);
	}
#else 
	WindowInitialisation initInfo;
	initInfo.windowTitle = "CSC8508 Game technology!";
	initInfo.width = wWidth;
	initInfo.height = wHeight;

	Window* w = Window::CreateGameWindow(initInfo);

	if (!w->HasInitialised())
		return -1;
	w->ShowOSPointer(true);
	w->LockMouseToWindow(true);
	w->GetTimer().GetTimeDeltaSeconds();

	NetworkedGame* g = new NetworkedGame();

	while (w->UpdateWindow() && !Window::GetKeyboard()->KeyDown(KeyCodes::ESCAPE))
	{
		float dt = w->GetTimer().GetTimeDeltaSeconds();
		w->SetTitle("Gametech frame time:" + std::to_string(std::roundf(1000.0f * dt)));
		g->UpdateGame(dt);
		Debug::UpdateRenderables(dt);
	}
	Window::DestroyGameWindow();
#endif
	
}