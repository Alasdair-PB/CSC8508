
#include "Window.h"
#include "Debug.h"

#include "TutorialGame.h"
#include "NetworkedGame.h"

using namespace NCL;
using namespace CSC8508;

#include <chrono>
#include <thread>
#include <sstream>


void UpdateWindow(Window* w, NetworkedGame* g)
{
	float dt = w->GetTimer().GetTimeDeltaSeconds();
	w->SetTitle("Gametech frame time:" + std::to_string(std::roundf(1000.0f * dt)));
	g->UpdateGame(dt);
}

int main(int argc, char** argv) 
{
	WindowInitialisation initInfo;
	initInfo.width		= 1920;
	initInfo.height		= 1200;
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