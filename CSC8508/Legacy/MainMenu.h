#pragma once
#include "PushdownMachine.h"
#include "PushdownState.h"
#include "Window.h"

#include "Debug.h"
#include "Controller.h"
#include <EOSInitialisationManager.h>


namespace NCL {
	namespace CSC8508 {

		typedef std::function<void()> StartClient;
		typedef std::function<void()> StartServer;
		typedef std::function<void()> StartOffline;
		typedef std::function<void()> StartEOS;
		typedef std::function<void()> StartEOSLobbyCreation;
		typedef std::function<void()> StartEOSLobbySearch;

		class MainMenu {

		public:			
			typedef std::function<void(bool state)> SetPauseGame;

			MainMenu(SetPauseGame setPauseFunc, StartClient startClient, StartServer startServer, StartOffline startOffline, StartEOS startEOS, StartEOSLobbyCreation startEOSLobbyCreation, StartEOSLobbySearch startEOSLobbySearch);
			~MainMenu();
			void Update(float dt);

			void SetMainMenuOption(int option) { mainMenuOption = option; } // Change this for each menu
			void SetEOSMenuOption(int option) { eosMenuOption = option; } // Change this for each menu

			SetPauseGame setPause;
			StartClient startClient;
			StartServer startServer;
			StartOffline startOffline;
			StartEOS startEOS;
			StartEOSLobbyCreation startEOSLobbyCreation;
			StartEOSLobbySearch startEOSLobbySearch;

		protected:
			PushdownMachine* machine = nullptr;
			const Controller* activeController = nullptr;
			
			void OnStateAwake();
			void OnStateAwakePause() { setPause(true); }
			void OnStateAwakeUnpause() { setPause(false); }

			PushdownState::PushdownResult IntroScreenOnUpdate(float dt, PushdownState** newState);
			PushdownState::PushdownResult LobbyScreenOnUpdate(float dt, PushdownState** newState);
			PushdownState::PushdownResult GameScreenOnUpdate(float dt, PushdownState** newState);
			PushdownState::PushdownResult PauseScreenOnUpdate(float dt, PushdownState** newState);
			PushdownState::PushdownResult LobbyDetailsOnUpdate(float dt, PushdownState** newState);

			enum menuOptions { none, startOfflineOpt, startServerOpt, startClientOpt, eosOption }; //Relates to menuOptions in MainMenu.h
			int mainMenuOption = 0;
			
			enum eosMenuOptions { eosNone, hostLobby, joinLobby }; //Relates to menuOptions in MainMenu.h
			int eosMenuOption = 0;
		};
	}
}
