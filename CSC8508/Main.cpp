
#include "Window.h"
#include "Debug.h"

#include "TutorialGame.h"
#include "NetworkedGame.h"

using namespace NCL;
using namespace CSC8508;

#ifdef USE_PS5
#include "./PS5/GameTechAGCRenderer.h"
#include "PS5Window.h"
#else
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
	r->Update(dt);
	r->Render();
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
	WindowInitialisation initInfo;
	initInfo.windowTitle = "CSC8508 Game technology!";

#ifdef USE_PS5
	std::unique_ptr<PS5::PS5Window> w = std::make_unique<PS5::PS5Window>(initInfo.windowTitle, 1920, 1080);
	std::unique_ptr<GameTechAGCRenderer> renderer = std::make_unique<GameTechAGCRenderer>(*(world.get()));
#else 
	initInfo.width = wWidth;
	initInfo.height = wHeight;

	Window* w = Window::CreateGameWindow(initInfo);
	std::unique_ptr<GameTechRenderer> renderer = std::make_unique<GameTechRenderer>(*(world.get()));
#endif
	NetworkedGame* g = new NetworkedGame(world.get(), renderer.get());
#ifndef USE_PS5
	if (!w->HasInitialised()) 
		return -1;
#endif // !USE_PS5

	w->ShowOSPointer(true);
	w->LockMouseToWindow(true);
	w->GetTimer().GetTimeDeltaSeconds();

	while (w->UpdateWindow() && !Window::GetKeyboard()->KeyDown(KeyCodes::ESCAPE))
	{
		float dt = w->GetTimer().GetTimeDeltaSeconds();
		world->UpdateWorld(dt);
		w->SetTitle("Gametech frame time:" + std::to_string(std::roundf(1000.0f * dt)));
		g->UpdateGame(dt);
		renderer->Update(dt);
		renderer->Render();
		Debug::UpdateRenderables(dt);
	}
	Window::DestroyGameWindow();
	
}