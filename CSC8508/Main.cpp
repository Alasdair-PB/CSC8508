#include "Debug.h"

#if EDITOR
#include "EditorGame.h"
#else 
#include "TutorialGame.h"
#include "NetworkedGame.h"
#endif

#include <chrono>
#include <thread>
#include <sstream>
#include <functional>
#include <cstdlib>

#ifdef USE_PS5
#include "./PS5/GameTechAGCRenderer.h"
#include "PS5Window.h"
using namespace NCL;
using namespace PS5;

size_t sceUserMainThreadStackSize = 2 * 1024 * 1024;
extern const char sceUserMainThreadName[] = "TeamProjectGameMain";
int sceUserMainThreadPriority = SCE_KERNEL_PRIO_FIFO_DEFAULT;
size_t sceLibcHeapSize = 256 * 1024 * 1024;
#else
#include "Window.h"
#include "GameTechRenderer.h"
using namespace NCL;
using namespace CSC8508;
#endif

#if EDITOR
void UpdateWindow(Window* w, EditorGame* g)
#else 
void UpdateWindow(Window* w, NetworkedGame* g)
#endif
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

#if EDITOR
	EditorGame* g = new EditorGame();
#else 
	NetworkedGame* g = new NetworkedGame();
#endif

	while (w->UpdateWindow() && !Window::GetKeyboard()->KeyDown(KeyCodes::ESCAPE))
	{
		float dt = w->GetTimer().GetTimeDeltaSeconds();
		g->UpdateGame(dt);
		Debug::UpdateRenderables(dt);
	}
	Window::DestroyGameWindow();
#endif
	
}