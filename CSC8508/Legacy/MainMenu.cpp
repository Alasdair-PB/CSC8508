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

#if PS5
		MainMenu::MainMenu(SetPauseGame setPauseFunc,
			StartClient startClient,
			StartServer startServer,
			StartOffline startOffline)
		{
			setPause = setPauseFunc;
			this->startClient = startClient;
			this->startServer = startServer;
			this->startOffline = startOffline;
#else
		MainMenu::MainMenu(SetPauseGame setPauseFunc,
			StartClient startClient,
			StartServer startServer,
			StartOffline startOffline,
			StartEOS startEOS,
			StartEOSLobbyCreation startEOSLobbyCreation,
			StartEOSLobbySearch startEOSLobbySearch,
			StartEOSLobbyUpdate startEOSLobbyUpdate,
			GetStringFunc getOwnerIP,
			GetStringFunc getLobbyID,
			GetIntFunc getPlayerCount,
			EOSStartAsHost eosStartAsHost,
			EOSStartAsHost eosStartAsJoin)
		{
			setPause = setPauseFunc;
			this->startClient = startClient;
			this->startServer = startServer;
			this->startOffline = startOffline;
			this->startEOS = startEOS;
			this->startEOSLobbyCreation = startEOSLobbyCreation;
			this->startEOSLobbySearch = startEOSLobbySearch;
			this->startEOSLobbyUpdate = startEOSLobbyUpdate;
			getOwnerIPFunc = getOwnerIP;
			getLobbyIDFunc = getLobbyID;
			getPlayerCountFunc = getPlayerCount;
			this->EOSStartAsHostFunc = eosStartAsHost;
			this->EOSStartAsJoinFunc = eosStartAsJoin;
#endif
			

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

#if !PS5
			if (eosFlowFinished)
			{
				std::cout << "Popping Success";
				return PushdownState::Pop;
			}
#endif

			if (mainMenuOption == startClientOpt) {
				std::cout << "Start Client Pressed";
				setPause(false);
				startClient();
				return PushdownState::Pop;
			}
			if (mainMenuOption == startServerOpt) {
				setPause(false);
				startServer();
				return PushdownState::Pop;
			}
			if (mainMenuOption == startOfflineOpt) {
				setPause(false);
				startOffline();
				return PushdownState::Pop;
			}
#if !PS5
			if (mainMenuOption == eosOption) {
				startEOS();
				*newState = new OverlayScreen(
					[&]() -> void { this->OnStateAwakePause(); },
					[&](float dt, PushdownState** newState) -> PushdownState::PushdownResult {
						return this->LobbyScreenOnUpdate(dt, newState);
					}
				);
				return PushdownState::Push;
			}
#endif
			return PushdownState::NoChange;
		}

#if !PS5
		PushdownState::PushdownResult MainMenu::LobbyScreenOnUpdate(float dt, PushdownState** newState)
		{
			Debug::Print("Duplicate Main Menu", Vector2(5, 85));

			if (eosFlowFinished)
			{
				return PushdownState::Pop;
			}

			if (eosMenuOption == hostLobby) {
				setPause(false);
				startEOSLobbyCreation();

				*newState = new OverlayScreen(
					[&]() -> void { this->OnStateAwakePause(); },
					[&](float dt, PushdownState** newState) -> PushdownState::PushdownResult {
						return this->LobbyDetailsOnUpdate(dt, newState);
					}
				);
				return PushdownState::Push;
			}
			if (eosMenuOption == joinLobby) {
				setPause(false);
				startEOSLobbySearch(lobbyCodeInput);

				*newState = new OverlayScreen(
					[&]() -> void { this->OnStateAwakePause(); },
					[&](float dt, PushdownState** newState) -> PushdownState::PushdownResult {
						return this->LobbyDetailsOnUpdate(dt, newState);
					}
				);
				return PushdownState::Push;
			}
			return PushdownState::NoChange;
		}

		PushdownState::PushdownResult MainMenu::LobbyDetailsOnUpdate(float dt, PushdownState** newState)
		{
			lobbyUpdateTimer += dt;
			if (lobbyUpdateTimer >= updateInterval)
			{
				startEOSLobbyUpdate();
				lobbyUpdateTimer = 0.0f;
			}

			if (eosLobbyOption == startGameAsHost)
			{
				setPause(false);
				EOSStartAsHostFunc();
				eosFlowFinished = true;
				return PushdownState::Pop;
			}
			
			if (eosLobbyOption == startGameAsJoin)
			{
				setPause(false);
				EOSStartAsJoinFunc();
				eosFlowFinished = true;
				return PushdownState::Pop;
			}

			return PushdownState::NoChange;
		}

		PushdownState::PushdownResult LobbyScreenOnUpdate(float dt, PushdownState** newState) {
			Debug::Print("Waiting for players...", Vector2(5, 80));

			return PushdownState::NoChange;
		}

#endif

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