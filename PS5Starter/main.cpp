#include "PS5Window.h"
#include "../CSC8508/NetworkedGame.h"
#include "../CSC8508CoreClasses/PhysicsSystem.h"
#include "../CSC8508CoreClasses/GameWorld.h"
#include "../CSC8508/GameTechRendererInterface.h"
#include "GameTechAGCRenderer.h"

using namespace NCL;
using namespace PS5;

size_t sceUserMainThreadStackSize = 2 * 1024 * 1024;
extern const char sceUserMainThreadName[] = "TeamProjectGameMain";
int sceUserMainThreadPriority = SCE_KERNEL_PRIO_FIFO_DEFAULT;
size_t sceLibcHeapSize = 256 * 1024 * 1024;

int main() {
	std::unique_ptr<PS5Window>		window	= std::make_unique<PS5Window>("Hello!", 1920, 1080);
	std::unique_ptr<GameWorld>		world	= std::make_unique<GameWorld>();
	std::unique_ptr<PhysicsSystem>	physics	= std::make_unique<PhysicsSystem>(*world);

#ifdef USEVULKAN
	std::unique_ptr<GameTechVulkanRenderer> renderer = std::make_unique<GameTechVulkanRenderer>(*world);
#elif USEAGC
	std::unique_ptr<GameTechAGCRenderer> renderer = std::make_unique<GameTechAGCRenderer>(*world);
	PS5Controller* c = window->GetController();
#else 
	GameTechRenderer* renderer = new GameTechRenderer(*world);
#endif

	c->MapAxis(0, "LeftX");
	c->MapAxis(1, "LeftY");

	c->MapAxis(2, "RightX");
	c->MapAxis(3, "RightY");

	c->MapAxis(4, "DX");
	c->MapAxis(5, "DY");

	c->MapButton(0, "Triangle");
	c->MapButton(1, "Circle");
	c->MapButton(2, "Cross");
	c->MapButton(3, "Square");

	//These are the axis/button aliases the inbuilt camera class reads from:
	c->MapAxis(0, "XLook");
	c->MapAxis(1, "YLook");

	c->MapAxis(2, "Sidestep");
	c->MapAxis(3, "Forward");

	c->MapButton(0, "Up");
	c->MapButton(2, "Down");

	std::unique_ptr<TutorialGame> g = std::make_unique<TutorialGame>(world.get(), renderer.get());

	while (window->UpdateWindow()) {
		float dt = window->GetTimer().GetTimeDeltaSeconds();
		g->UpdateGame(dt);
		renderer->Update(dt);
		renderer->Render();
		//Debug::UpdateRenderables(dt);
	}

	return 0;
}