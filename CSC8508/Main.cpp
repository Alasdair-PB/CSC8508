#include "Window.h"

#include "Debug.h"

#include "StateMachine.h"
#include "StateTransition.h"
#include "State.h"

#include "GameServer.h"
#include "GameClient.h"

#include "NavigationGrid.h"
#include "NavigationMesh.h"

#include "TutorialGame.h"
#include "NetworkedGame.h"

#include "PushdownMachine.h"

#include "PushdownState.h"

#include "BehaviourNode.h"
#include "BehaviourSelector.h"
#include "BehaviourSequence.h"
#include "BehaviourAction.h"

#include "RenderObject.h"

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

#pragma region TestStateMachine

void TestStateMachine() {
	StateMachine* testMachine = new StateMachine();
	int data = 0;

	State* A = new State([&](float dt) -> void {
		std::cout << "I’m in state A!\n";
		data++;
		});

	State* B = new State([&](float dt) -> void {
		std::cout << "I’m in state B!\n";
		data--;
		});

	StateTransition* stateAB = new StateTransition(A, B, [&](void)-> bool {return data > 10;});
	StateTransition* stateBA = new StateTransition(B, A, [&](void)-> bool {return data < 0;});

	testMachine->AddState(A);
	testMachine->AddState(B);
	testMachine->AddTransition(stateAB);
	testMachine->AddTransition(stateBA);

	for (int i = 0; i < 100; ++i) {
		testMachine->Update(1.0f);
	}
}
#pragma endregion

#pragma region PathFinding


/*void TestGridPathfinding()
{
	NavigationGrid grid("TestGrid1.txt");
	NavigationPath outPath;

	Vector3 startPos = Vector3(0, 0, 0);
	Vector3 endPos = Vector3(25, 0, 5);

	bool found = grid.FindPath(startPos, endPos, outPath);
	Vector3 pos;
	if (!found)
		std::cout << "Path not found" << std::endl;

	while (outPath.PopWaypoint(pos)) {
		testNodes.push_back(pos);
	}
}*/



#pragma endregion

#pragma region BehaviourTrees

void TestBehaviourTree() 
{

	float behaviourTimer;
	float distanceToTarget;

	BehaviourAction* findKey = new BehaviourAction("Find Key",
		[&](float dt, BehaviourState state) -> BehaviourState {
			if (state == Initialise) {
				std::cout << "Looking for a key!\n";
				behaviourTimer = rand() % 100;
				state = Ongoing;
			}
			else if (state == Ongoing) {
				behaviourTimer -= dt;
				if (behaviourTimer <= 0.0f) {
					std::cout << "Found a key!\n";
					return Success;
				}
			}
			return state; // will be 'ongoing' until success
		}
	);

	BehaviourAction* goToRoom = new BehaviourAction("Go To Room",
		[&](float dt, BehaviourState state) -> BehaviourState {
			if (state == Initialise) {
				std::cout << "Going to the loot room!\n";
				state = Ongoing;
			}
			else if (state == Ongoing) {
				distanceToTarget -= dt;
				if (distanceToTarget <= 0.0f) {
					std::cout << "Reached room!\n";
					return Success;
				}
			}
			return state; // will be 'ongoing' until success
		}
	);

	BehaviourAction* openDoor = new BehaviourAction("Open Door",
		[&](float dt, BehaviourState state) -> BehaviourState {
			if (state == Initialise) {
				std::cout << "Opening Door!\n";
				return Success;
			}
			return state;
		}
	);

	BehaviourAction* lookForTreasure = new BehaviourAction(
		"Look For Treasure",
		[&](float dt, BehaviourState state) -> BehaviourState {
			if (state == Initialise) {
				std::cout << "Looking for treasure!\n";
				return Ongoing;
			}
			else if (state == Ongoing) {
				bool found = rand() % 2;
				if (found) {
					std::cout << "I found some treasure!\n";
					return Success;
				}
				std::cout << "No treasure in here...\n";
				return Failure;
			}
			return state;
		}
	);

	BehaviourAction* lookForItems = new BehaviourAction(
		"Look For Items",
		[&](float dt, BehaviourState state) -> BehaviourState {
			if (state == Initialise) {
				std::cout << "Looking for items!\n";
				return Ongoing;
			}
			else if (state == Ongoing) {
				bool found = rand() % 2;
				if (found) {
					std::cout << "I found some items!\n";
					return Success;
				}
				std::cout << "No items in here...\n";
				return Failure;
			}
			return state;
		}
	);

	BehaviourSequence* sequence = new BehaviourSequence("Room Sequence");
	sequence->AddChild(findKey);
	sequence->AddChild(goToRoom);
	sequence->AddChild(openDoor);

	BehaviourSelector* selection = new BehaviourSelector("Loot Selection");
	selection->AddChild(lookForTreasure);
	selection->AddChild(lookForItems);

	BehaviourSequence* rootSequence = new BehaviourSequence("Root Sequence");
	rootSequence->AddChild(sequence);
	rootSequence->AddChild(selection);

	for (int i = 0; i < 5; ++i) {
		rootSequence->Reset();
		behaviourTimer = 0.0f;
		distanceToTarget = rand() % 250;
		BehaviourState state = Ongoing;
		std::cout << "We're going on an adventure!\n";
		while (state == Ongoing) {
			state = rootSequence->Execute(1.0f); // fake dt
		}
		if (state == Success) {
			std::cout << "What a successful adventure!\n";
		}
		else if (state == Failure) {
			std::cout << "What a waste of time!\n";
		}
	}
	std::cout << "All done!\n";
}

#pragma endregion

/*
void UpdateWindow(Window* w, NetworkedGame* g, GameTechAGCRenderer* r)
{
	float dt = w->GetTimer().GetTimeDeltaSeconds();
	w->SetTitle("Gametech frame time:" + std::to_string(std::roundf(1000.0f * dt)));
	g->UpdateGame(dt);
	r->Update(dt);
	r->Render();
	Debug::UpdateRenderables(dt);
}
*/

int main(int argc, char** argv) 
{
	std::unique_ptr<GameWorld>	world = std::make_unique<GameWorld>();

#ifdef USE_PS5
	std::unique_ptr<PS5::PS5Window> w = std::make_unique<PS5::PS5Window>("Hello!", 1920, 1080);
	std::unique_ptr<GameTechAGCRenderer> renderer = std::make_unique<GameTechAGCRenderer>(*(world.get()));
#else 
	WindowInitialisation initInfo;
	initInfo.width = 1920;
	initInfo.height = 1200;
	initInfo.windowTitle = "CSC8508 Game technology!";

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