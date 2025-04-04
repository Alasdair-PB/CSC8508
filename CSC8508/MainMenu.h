#pragma once
#include "PushdownMachine.h"
#include "PushdownState.h"
#include "Window.h"

#include "Debug.h"
#include "Controller.h"
#include "functional"

namespace NCL {
	namespace CSC8508 {

		typedef std::function<void()> StartClient;
		typedef std::function<void()> StartServer;
		typedef std::function<void()> StartOffline;

#if EOSBUILD
		typedef std::function<void()> StartEOS;
		typedef std::function<void()> StartEOSLobbyCreation;
		typedef std::function<void(std::string)> StartEOSLobbySearch;
		typedef std::function<void()> StartEOSLobbyUpdate;
		typedef std::function<void()> EOSStartAsHost;
		typedef std::function<void(const std::string&)> EOSStartAsJoin;
		using GetStringFunc = std::function<std::string()>;
		using GetIntFunc = std::function<int()>;
#endif

		class MainMenu {

		public:			
			typedef std::function<void(bool state)> SetPauseGame;

#if !EOSBUILD
			MainMenu(SetPauseGame setPauseFunc,
				StartClient startClient,
				StartServer startServer,
				StartOffline startOffline);
#else
			MainMenu(SetPauseGame setPauseFunc,
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
				EOSStartAsJoin eosStartAsJoin);
#endif

			~MainMenu();
			void Update(float dt);

			void SetMainMenuOption(int option) { mainMenuOption = option; } // Change this for each menu

#if EOSBUILD
			void SetEOSMenuOption(int option) { eosMenuOption = option; } // Change this for each menu
			void SetEOSLobbyOption(int option) { eosLobbyOption = option; } // Change this for each menu
#endif

			float lobbyUpdateTimer = 10.0f;
			const float updateInterval = 10.0f; // 3 seconds

			SetPauseGame setPause;
			StartClient startClient;
			StartServer startServer;
			StartOffline startOffline;

#if EOSBUILD
			StartEOS startEOS;
			StartEOSLobbyCreation startEOSLobbyCreation;
			StartEOSLobbySearch startEOSLobbySearch;
			StartEOSLobbyUpdate startEOSLobbyUpdate;
			EOSStartAsHost EOSStartAsHostFunc;
			EOSStartAsJoin EOSStartAsJoinFunc;

			GetStringFunc getOwnerIPFunc;
			GetStringFunc getLobbyIDFunc;
			GetIntFunc getPlayerCountFunc;

			std::string lobbyCodeInput;
#endif

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
			
#if EOSBUILD
			enum eosMenuOptions { eosNone, hostLobby, joinLobby }; //Relates to menuOptions in MainMenu.h
			int eosMenuOption = 0;

			enum eosLobbyOptions {eosLobbyNone, startGameAsHost, startGameAsJoin};
			int eosLobbyOption = 0;

			bool eosFlowFinished = false;
#endif
		};
	}
}