
#include "Window.h"
#include "Debug.h"

#include "TutorialGame.h"
#include "NetworkedGame.h"

using namespace NCL;
using namespace CSC8508;

#include <chrono>
#include <thread>
#include <sstream>
#include <cstdlib>


void UpdateWindow(Window* w, NetworkedGame* g)
{
	float dt = w->GetTimer().GetTimeDeltaSeconds();
	w->SetTitle("Gametech frame time:" + std::to_string(std::roundf(1000.0f * dt)));
	g->UpdateGame(dt);
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
	initInfo.width		= wWidth;
	initInfo.height		= wHeight; 
	initInfo.windowTitle = "CSC8508 Game technology!";

	Window* w = Window::CreateGameWindow(initInfo);
	NetworkedGame* g = new NetworkedGame();

	if (!w->HasInitialised()) return -1;
	w->ShowOSPointer(true);
	w->LockMouseToWindow(true);
	w->GetTimer().GetTimeDeltaSeconds();

	while (w->UpdateWindow() && !Window::GetKeyboard()->KeyDown(KeyCodes::ESCAPE))
	{
		UpdateWindow(w, g);
	}
	Window::DestroyGameWindow();
	
}