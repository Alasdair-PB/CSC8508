#include "MainMenu.h";

//using namespace NCL;

namespace NCL {
	namespace CSC8508 {

		class OverlayScreen : public PushdownState
		{
		public:

			OverlayScreen(OnAwakeFunction onAwake, OnUpdateFunction onUpdate) : PushdownState(onAwake, onUpdate) {};

			PushdownResult OnUpdate(float dt, PushdownState** newState) override
			{
				return PushdownState::OnUpdate(dt, newState);
			}
			void OnAwake() override
			{
				PushdownState::OnAwake();
			}
		};

		MainMenu::MainMenu(SetPauseGame setPauseFunc, StartClient startClient, StartServer startServer, StartOffline startOffline, StartEOS startEOS)
		{
			setPause = setPauseFunc;
			this->startClient = startClient;
			this->startServer = startServer;
			this->startOffline = startOffline;
			this->startEOS = startEOS;

			machine = new PushdownMachine(new OverlayScreen(
				[&]() -> void { this->OnStateAwake(); },
				[&](float dt, PushdownState** newState) -> PushdownState::PushdownResult {
					return this->GameScreenOnUpdate(dt, newState);
				}
			));

			machine->PushStateToStack(new OverlayScreen(
				[&]() -> void { this->OnStateAwakePause(); },
				[&](float dt, PushdownState** newState) -> PushdownState::PushdownResult {
					return this->IntroScreenOnUpdate(dt, newState);
				}
			));
		}

		void MainMenu::OnStateAwake()
		{
			setPause(false);
		}

	
		PushdownState::PushdownResult MainMenu::GameScreenOnUpdate(float dt, PushdownState** newState)
		{
			Debug::Print("Game Screen", Vector2(5, 85));

			if (Window::GetKeyboard()->KeyPressed(KeyCodes::P)) {
				*newState = new OverlayScreen(
					[&]()-> void { this->OnStateAwakePause(); },
					[&](float dt, PushdownState** newState) -> PushdownState::PushdownResult {
						return this->PauseScreenOnUpdate(dt, newState);
					}
				);
				return PushdownState::PushdownResult::Push;
			}
			return PushdownState::PushdownResult::NoChange;
		}

		PushdownState::PushdownResult MainMenu::PauseScreenOnUpdate(float dt, PushdownState** newState)
		{
			Debug::Print("Paused", Vector2(5, 85));

			if (Window::GetKeyboard()->KeyPressed(KeyCodes::P))
			{
				setPause(false);
				return PushdownState::PushdownResult::Pop;
			}
			return PushdownState::PushdownResult::NoChange;
		}


		PushdownState::PushdownResult MainMenu::IntroScreenOnUpdate(float dt, PushdownState** newState)
		{
			Debug::Print("Main Menu", Vector2(5, 85));
			/*Debug::Print(" C->Start as client", Vector2(5, 65));
			Debug::Print("V->Start as server", Vector2(5, 55));
			Debug::Print("B->Start Offline", Vector2(5, 45));*/

			if (menuOption == startClientOpt) {
				setPause(false);
				startClient();
				return PushdownState::PushdownResult::Pop;
			}
			if (menuOption == startServerOpt) {
				setPause(false);
				startServer();
				return PushdownState::PushdownResult::Pop;
			}
			if (menuOption == startOfflineOpt) {
				setPause(false);
				startOffline();
				return PushdownState::PushdownResult::Pop;
			}
			if (menuOption == hostRoomOpt)
			{
				setPause(false);
				std::cout << "Here";
				startEOS();
				return PushdownState::PushdownResult::Pop;
			}
			return PushdownState::PushdownResult::NoChange;
		}

		MainMenu::~MainMenu() {
			delete activeController;
		}


		void MainMenu::Update(float dt)
		{ 
			if (!machine->Update(dt))
				return;
		}
	}
}